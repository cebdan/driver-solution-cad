#include <iostream>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>

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
    double getA() const { return a_; }
    double getB() const { return b_; }
    double getC() const { return c_; }
    
    void getTransformationMatrix(double matrix[16]) const {
        // Simple identity matrix with translation
        for (int i = 0; i < 16; i++) matrix[i] = 0.0;
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0;
        matrix[12] = x_;
        matrix[13] = y_;
        matrix[14] = z_;
    }
    
    static SimpleCS GlobalCS() {
        SimpleCS global(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        global.setName("Global CS");
        return global;
    }
    
private:
    double x_, y_, z_, a_, b_, c_;
    std::string name_;
};

class Point3D {
public:
    Point3D() : x_(0.0), y_(0.0), z_(0.0), cs_(nullptr) {}
    Point3D(double x, double y, double z) : x_(x), y_(y), z_(z), cs_(nullptr) {}
    Point3D(double x, double y, double z, SimpleCS* cs) : x_(x), y_(y), z_(z), cs_(cs) {}
    
    void setX(double x) { x_ = x; }
    void setY(double y) { y_ = y; }
    void setZ(double z) { z_ = z; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    
    void setCoordinateSystem(SimpleCS* cs) { cs_ = cs; }
    SimpleCS* getCoordinateSystem() const { return cs_; }
    bool hasCoordinateSystem() const { return cs_ != nullptr; }
    
    Point3D operator+(const Point3D& other) const {
        return Point3D(x_ + other.x_, y_ + other.y_, z_ + other.z_);
    }
    
    Point3D operator-(const Point3D& other) const {
        return Point3D(x_ - other.x_, y_ - other.y_, z_ - other.z_);
    }
    
    Point3D operator*(double scalar) const {
        return Point3D(x_ * scalar, y_ * scalar, z_ * scalar);
    }
    
    double distance(const Point3D& other) const {
        double dx = x_ - other.x_;
        double dy = y_ - other.y_;
        double dz = z_ - other.z_;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    double distanceToOrigin() const {
        return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
    }
    
    double dot(const Point3D& other) const {
        return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
    }
    
    Point3D cross(const Point3D& other) const {
        return Point3D(
            y_ * other.z_ - z_ * other.y_,
            z_ * other.x_ - x_ * other.z_,
            x_ * other.y_ - y_ * other.x_
        );
    }
    
    double length() const {
        return distanceToOrigin();
    }
    
    Point3D transformToGlobal() const {
        if (!cs_) return *this;
        return Point3D(cs_->getX() + x_, cs_->getY() + y_, cs_->getZ() + z_);
    }
    
    void projectTo2D(double& x, double& y, int axis = 2) const {
        switch (axis) {
            case 0: x = y_; y = z_; break;
            case 1: x = x_; y = z_; break;
            case 2: default: x = x_; y = y_; break;
        }
    }
    
    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Point3D(" << x_ << ", " << y_ << ", " << z_;
        if (cs_) {
            oss << ", CS: " << cs_->getName();
        }
        oss << ")";
        return oss.str();
    }
    
private:
    double x_, y_, z_;
    SimpleCS* cs_;
    friend std::ostream& operator<<(std::ostream& os, const Point3D& p) {
        return os << p.toString();
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== 3D Point Demo ===" << std::endl << std::endl;
    
    // Create Global CS
    SimpleCS globalCS = SimpleCS::GlobalCS();
    
    // Create points without CS
    Point3D p1(10.0, 20.0, 30.0);
    Point3D p2(40.0, 50.0, 60.0);
    
    std::cout << "Point 1: " << p1.toString() << std::endl;
    std::cout << "Point 2: " << p2.toString() << std::endl;
    std::cout << "Distance between points: " << p1.distance(p2) << std::endl;
    std::cout << "Distance p1 to origin: " << p1.distanceToOrigin() << std::endl;
    
    // Vector operations
    double dot_product = p1.dot(p2);
    Point3D cross_product = p1.cross(p2);
    std::cout << "\nDot product: " << dot_product << std::endl;
    std::cout << "Cross product: " << cross_product.toString() << std::endl;
    std::cout << "Length of p1: " << p1.length() << std::endl;
    
    // Create point with CS
    Point3D p3(5.0, 10.0, 15.0, &globalCS);
    std::cout << "\nPoint 3 (with CS): " << p3.toString() << std::endl;
    
    // Transform to global coordinates
    Point3D p3_global = p3.transformToGlobal();
    std::cout << "Point 3 in global coordinates: " << p3_global.toString() << std::endl;
    
    // Create local CS
    SimpleCS localCS(100.0, 200.0, 300.0, 0.0, 0.0, 0.0);
    localCS.setName("Local CS");
    
    Point3D p4(15.0, 25.0, 35.0, &localCS);
    std::cout << "\nPoint 4 (with Local CS): " << p4.toString() << std::endl;
    
    Point3D p4_global = p4.transformToGlobal();
    std::cout << "Point 4 in global coordinates: " << p4_global.toString() << std::endl;
    
    // Point operations
    Point3D p5 = p1 + p2;
    std::cout << "\nPoint 1 + Point 2 = " << p5.toString() << std::endl;
    
    Point3D p6 = p2 - p1;
    std::cout << "Point 2 - Point 1 = " << p6.toString() << std::endl;
    
    Point3D p7 = p1 * 2.5;
    std::cout << "Point 1 * 2.5 = " << p7.toString() << std::endl;
    
    // Projection to 2D
    double proj_x, proj_y;
    p1.projectTo2D(proj_x, proj_y, 2); // Project to XY plane
    std::cout << "\nPoint 1 projected to XY plane: (" << proj_x << ", " << proj_y << ")" << std::endl;
    
    p1.projectTo2D(proj_x, proj_y, 1); // Project to XZ plane
    std::cout << "Point 1 projected to XZ plane: (" << proj_x << ", " << proj_y << ")" << std::endl;
    
    return 0;
}

