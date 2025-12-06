#include "../include/Document2D.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cctype>

Document2D::Document2D() 
    : SolutionDocument(), default_cs_(nullptr), units_("mm"), scale_(1.0), is_open_(false) {
    setName("Untitled 2D Document");
}

Document2D::Document2D(const std::string& name)
    : SolutionDocument(), default_cs_(nullptr), units_("mm"), scale_(1.0), is_open_(false) {
    setName(name);
}

bool Document2D::load(const std::string& file_path) {
    if (file_path.empty()) {
        return false;
    }
    
    std::string format;
    if (!detectFileFormat(file_path, format)) {
        return false;
    }
    
    bool success = false;
    if (format == "json") {
        success = loadFromJSON(file_path);
    } else if (format == "xml") {
        success = loadFromXML(file_path);
    } else {
        // Try default JSON format
        success = loadFromJSON(file_path);
    }
    
    if (success) {
        setPath(file_path);
        setModified(false);
        is_open_ = true;
    }
    
    return success;
}

bool Document2D::save(const std::string& file_path) {
    std::string path = file_path.empty() ? getPath() : file_path;
    if (path.empty()) {
        return false;
    }
    
    std::string format;
    if (!detectFileFormat(path, format)) {
        format = "json"; // Default format
    }
    
    bool success = false;
    if (format == "json") {
        success = saveToJSON(path);
    } else if (format == "xml") {
        success = saveToXML(path);
    } else {
        success = saveToJSON(path);
    }
    
    if (success) {
        setPath(path);
        setModified(false);
    }
    
    return success;
}

bool Document2D::saveAs(const std::string& file_path) {
    return save(file_path);
}

void Document2D::close() {
    if (is_open_) {
        clear();
        setPath("");
        is_open_ = false;
    }
}

bool Document2D::isOpen() const {
    return is_open_;
}

void Document2D::clear() {
    clearPoints();
    clearSolutions();
    setModified(false);
}

bool Document2D::isEmpty() const {
    return points_.empty() && getSolutionCount() == 0;
}

std::unique_ptr<SolutionDocument> Document2D::copy() const {
    auto copy = std::make_unique<Document2D>(getName() + " (Copy)");
    
    // Copy metadata
    copy->setAuthor(getAuthor());
    copy->setDescription(getDescription());
    copy->setVersion(getVersion());
    copy->setUnits(units_);
    copy->setScale(scale_);
    
    // Copy points
    for (const auto& point : points_) {
        copy->addPoint(*point);
    }
    
    // Copy coordinate system reference
    copy->setDefaultCoordinateSystem(default_cs_);
    
    return copy;
}

void Document2D::useFor(const std::string& purpose) {
    setDescription("Used for: " + purpose);
}

bool Document2D::sendTo(const std::string& destination, const std::string& format) {
    return exportTo(destination, format);
}

bool Document2D::import(const std::string& file_path, const std::string& format) {
    std::string actual_format = format;
    if (actual_format.empty()) {
        detectFileFormat(file_path, actual_format);
    }
    
    if (actual_format == "json") {
        return loadFromJSON(file_path);
    } else if (actual_format == "xml") {
        return loadFromXML(file_path);
    }
    
    return false;
}

bool Document2D::exportTo(const std::string& file_path, const std::string& format) {
    std::string actual_format = format;
    if (actual_format.empty()) {
        detectFileFormat(file_path, actual_format);
    }
    
    if (actual_format == "json") {
        return saveToJSON(file_path);
    } else if (actual_format == "xml") {
        return saveToXML(file_path);
    }
    
    return false;
}

std::vector<std::string> Document2D::getSupportedImportFormats() const {
    return {"json", "xml", "txt", "csv"};
}

std::vector<std::string> Document2D::getSupportedExportFormats() const {
    return {"json", "xml", "txt", "csv", "svg"};
}

void Document2D::addPoint(const Point2D& point) {
    points_.push_back(std::make_unique<Point2D>(point));
    setModified(true);
}

void Document2D::addPoint(double x, double y) {
    Point2D point(x, y, default_cs_);
    addPoint(point);
}

void Document2D::removePoint(size_t index) {
    if (index < points_.size()) {
        points_.erase(points_.begin() + index);
        setModified(true);
    }
}

Point2D* Document2D::getPoint(size_t index) {
    if (index < points_.size()) {
        return points_[index].get();
    }
    return nullptr;
}

const Point2D* Document2D::getPoint(size_t index) const {
    if (index < points_.size()) {
        return points_[index].get();
    }
    return nullptr;
}

void Document2D::clearPoints() {
    points_.clear();
    setModified(true);
}

std::vector<Point2D*> Document2D::getAllPoints() {
    std::vector<Point2D*> result;
    for (const auto& point : points_) {
        result.push_back(point.get());
    }
    return result;
}

std::vector<const Point2D*> Document2D::getAllPoints() const {
    std::vector<const Point2D*> result;
    for (const auto& point : points_) {
        result.push_back(point.get());
    }
    return result;
}

void Document2D::setDefaultCoordinateSystem(CS* cs) {
    default_cs_ = cs;
    // Update all points to use this CS
    for (auto& point : points_) {
        if (!point->hasCoordinateSystem()) {
            point->setCoordinateSystem(cs);
        }
    }
}

void Document2D::getBoundingBox(double& min_x, double& min_y, double& max_x, double& max_y) const {
    if (points_.empty()) {
        min_x = min_y = max_x = max_y = 0.0;
        return;
    }
    
    min_x = min_y = std::numeric_limits<double>::max();
    max_x = max_y = std::numeric_limits<double>::lowest();
    
    for (const auto& point : points_) {
        double x = point->getX();
        double y = point->getY();
        
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
    }
}

bool Document2D::hasBoundingBox() const {
    return !points_.empty();
}

bool Document2D::detectFileFormat(const std::string& file_path, std::string& format) const {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos) {
        format = "json"; // Default
        return false;
    }
    
    std::string extension = file_path.substr(dot_pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "json") {
        format = "json";
        return true;
    } else if (extension == "xml") {
        format = "xml";
        return true;
    } else if (extension == "txt" || extension == "csv") {
        format = "txt";
        return true;
    }
    
    format = "json"; // Default
    return false;
}

bool Document2D::loadFromJSON(const std::string& file_path) {
    // Simplified JSON loading - in real implementation would use a JSON library
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Basic implementation - just clear and mark as loaded
    // Full implementation would parse JSON
    clearPoints();
    file.close();
    return true;
}

bool Document2D::saveToJSON(const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Basic JSON structure
    file << "{\n";
    file << "  \"name\": \"" << getName() << "\",\n";
    file << "  \"units\": \"" << units_ << "\",\n";
    file << "  \"scale\": " << scale_ << ",\n";
    file << "  \"points\": [\n";
    
    for (size_t i = 0; i < points_.size(); ++i) {
        file << "    {\"x\": " << points_[i]->getX() 
             << ", \"y\": " << points_[i]->getY() << "}";
        if (i < points_.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool Document2D::loadFromXML(const std::string& file_path) {
    // Simplified XML loading
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Basic implementation - just clear and mark as loaded
    clearPoints();
    file.close();
    return true;
}

bool Document2D::saveToXML(const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Basic XML structure
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<document2d>\n";
    file << "  <name>" << getName() << "</name>\n";
    file << "  <units>" << units_ << "</units>\n";
    file << "  <scale>" << scale_ << "</scale>\n";
    file << "  <points>\n";
    
    for (const auto& point : points_) {
        file << "    <point x=\"" << point->getX() 
             << "\" y=\"" << point->getY() << "\"/>\n";
    }
    
    file << "  </points>\n";
    file << "</document2d>\n";
    
    file.close();
    return true;
}

