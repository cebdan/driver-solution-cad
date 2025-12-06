#include <iostream>
#include <string>
#include <cmath>

// Simplified versions for demo
class SimpleCS {
public:
    SimpleCS() : x_(0.0), y_(0.0), z_(0.0), a_(0.0), b_(0.0), c_(0.0), name_("CS") {}
    SimpleCS(double x, double y, double z, double a, double b, double c)
        : x_(x), y_(y), z_(z), a_(a), b_(b), c_(c), name_("CS") {}
    
    void setName(const std::string& name) { name_ = name; }
    std::string getName() const { return name_; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    
    static SimpleCS GlobalCS() {
        SimpleCS global(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        global.setName("Global CS");
        return global;
    }
    
private:
    double x_, y_, z_, a_, b_, c_;
    std::string name_;
};

class Point2D {
public:
    Point2D() : x_(0.0), y_(0.0), cs_(nullptr) {}
    Point2D(double x, double y) : x_(x), y_(y), cs_(nullptr) {}
    Point2D(double x, double y, SimpleCS* cs) : x_(x), y_(y), cs_(cs) {}
    
    void setX(double x) { x_ = x; }
    void setY(double y) { y_ = y; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    
    void setCoordinateSystem(SimpleCS* cs) { cs_ = cs; }
    SimpleCS* getCoordinateSystem() const { return cs_; }
    bool hasCoordinateSystem() const { return cs_ != nullptr; }
    
    Point2D operator+(const Point2D& other) const {
        return Point2D(x_ + other.x_, y_ + other.y_);
    }
    
    Point2D operator-(const Point2D& other) const {
        return Point2D(x_ - other.x_, y_ - other.y_);
    }
    
    double distance(const Point2D& other) const {
        double dx = x_ - other.x_;
        double dy = y_ - other.y_;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    double distanceToOrigin() const {
        return std::sqrt(x_ * x_ + y_ * y_);
    }
    
    Point2D transformToGlobal() const {
        if (!cs_) return *this;
        return Point2D(cs_->getX() + x_, cs_->getY() + y_);
    }
    
    std::string toString() const {
        std::string result = "Point2D(" + std::to_string(x_) + ", " + std::to_string(y_);
        if (cs_) {
            result += ", CS: " + cs_->getName();
        }
        result += ")";
        return result;
    }
    
private:
    double x_, y_;
    SimpleCS* cs_;
};

int main() {
    std::cout << "=== 2D Point Demo ===" << std::endl << std::endl;
    
    // Create Global CS
    SimpleCS globalCS = SimpleCS::GlobalCS();
    
    // Create points without CS
    Point2D p1(10.0, 20.0);
    Point2D p2(30.0, 40.0);
    
    std::cout << "Point 1: " << p1.toString() << std::endl;
    std::cout << "Point 2: " << p2.toString() << std::endl;
    std::cout << "Distance between points: " << p1.distance(p2) << std::endl;
    std::cout << "Distance p1 to origin: " << p1.distanceToOrigin() << std::endl;
    
    // Create point with CS
    Point2D p3(5.0, 5.0, &globalCS);
    std::cout << "\nPoint 3 (with CS): " << p3.toString() << std::endl;
    
    // Transform to global coordinates
    Point2D p3_global = p3.transformToGlobal();
    std::cout << "Point 3 in global coordinates: " << p3_global.toString() << std::endl;
    
    // Create local CS
    SimpleCS localCS(100.0, 200.0, 0.0, 0.0, 0.0, 0.0);
    localCS.setName("Local CS");
    
    Point2D p4(15.0, 25.0, &localCS);
    std::cout << "\nPoint 4 (with Local CS): " << p4.toString() << std::endl;
    
    Point2D p4_global = p4.transformToGlobal();
    std::cout << "Point 4 in global coordinates: " << p4_global.toString() << std::endl;
    
    // Point operations
    Point2D p5 = p1 + p2;
    std::cout << "\nPoint 1 + Point 2 = " << p5.toString() << std::endl;
    
    Point2D p6 = p2 - p1;
    std::cout << "Point 2 - Point 1 = " << p6.toString() << std::endl;
    
    return 0;
}

