#ifndef DOCUMENT_2D_H
#define DOCUMENT_2D_H

#include "SolutionDocument.h"
#include "2D_point.h"
#include <vector>
#include <memory>
#include <string>

class Document2D : public SolutionDocument {
public:
    Document2D();
    Document2D(const std::string& name);
    virtual ~Document2D() = default;
    
    // Implementation of pure virtual methods from SolutionDocument
    virtual bool load(const std::string& file_path) override;
    virtual bool save(const std::string& file_path = "") override;
    virtual bool saveAs(const std::string& file_path) override;
    virtual void close() override;
    virtual bool isOpen() const override;
    
    virtual void clear() override;
    virtual bool isEmpty() const override;
    
    virtual std::unique_ptr<SolutionDocument> copy() const override;
    virtual void useFor(const std::string& purpose) override;
    virtual bool sendTo(const std::string& destination, const std::string& format = "") override;
    
    virtual bool import(const std::string& file_path, const std::string& format = "") override;
    virtual bool exportTo(const std::string& file_path, const std::string& format = "") override;
    virtual std::vector<std::string> getSupportedImportFormats() const override;
    virtual std::vector<std::string> getSupportedExportFormats() const override;
    
    // 2D specific methods
    void addPoint(const Point2D& point);
    void addPoint(double x, double y);
    void removePoint(size_t index);
    Point2D* getPoint(size_t index);
    const Point2D* getPoint(size_t index) const;
    size_t getPointCount() const { return points_.size(); }
    void clearPoints();
    
    std::vector<Point2D*> getAllPoints();
    std::vector<const Point2D*> getAllPoints() const;
    
    // Coordinate system management
    void setDefaultCoordinateSystem(CS* cs);
    CS* getDefaultCoordinateSystem() const { return default_cs_; }
    
    // Bounding box
    void getBoundingBox(double& min_x, double& min_y, double& max_x, double& max_y) const;
    bool hasBoundingBox() const;
    
    // Document properties
    void setUnits(const std::string& units) { units_ = units; }
    std::string getUnits() const { return units_; }
    
    void setScale(double scale) { scale_ = scale; }
    double getScale() const { return scale_; }
    
protected:
    std::vector<std::unique_ptr<Point2D>> points_;
    CS* default_cs_;
    std::string units_;
    double scale_;
    bool is_open_;
    
    // Helper methods
    bool detectFileFormat(const std::string& file_path, std::string& format) const;
    bool loadFromJSON(const std::string& file_path);
    bool saveToJSON(const std::string& file_path) const;
    bool loadFromXML(const std::string& file_path);
    bool saveToXML(const std::string& file_path) const;
};

#endif // DOCUMENT_2D_H

