#ifndef POINT_2D_H
#define POINT_2D_H

#include "CS.h"
#include <memory>
#include <cmath>
#include <string>

class Point2D {
public:
    Point2D();
    Point2D(double x, double y);
    Point2D(double x, double y, CS* coordinate_system);
    virtual ~Point2D() = default;
    
    // Coordinate access
    void setX(double x) { x_ = x; }
    void setY(double y) { y_ = y; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    void set(double x, double y);
    
    // Coordinate system management
    void setCoordinateSystem(CS* cs);
    CS* getCoordinateSystem() const { return cs_; }
    bool hasCoordinateSystem() const { return cs_ != nullptr; }
    
    // Point operations
    Point2D add(const Point2D& other) const;
    Point2D subtract(const Point2D& other) const;
    Point2D multiply(double scalar) const;
    Point2D divide(double scalar) const;
    
    // Operators
    Point2D operator+(const Point2D& other) const;
    Point2D operator-(const Point2D& other) const;
    Point2D operator*(double scalar) const;
    Point2D operator/(double scalar) const;
    bool operator==(const Point2D& other) const;
    bool operator!=(const Point2D& other) const;
    
    // Distance calculations
    double distance(const Point2D& other) const;
    double distanceToOrigin() const;
    double distanceSquared(const Point2D& other) const;
    
    // Geometric operations
    double dot(const Point2D& other) const;
    double cross(const Point2D& other) const;
    double length() const;
    double lengthSquared() const;
    Point2D normalize() const;
    
    // Angle calculations
    double angle(const Point2D& other) const;
    double angleToOrigin() const;
    
    // Transformation
    Point2D transform(const CS& target_cs) const;
    Point2D transformToGlobal() const;
    Point2D rotate(double angle) const;
    Point2D translate(double dx, double dy) const;
    Point2D scale(double sx, double sy) const;
    
    // Conversion to 3D (using z=0)
    void to3D(double& x, double& y, double& z) const;
    
    // Validation
    bool isValid() const;
    bool isZero() const;
    
    // String representation
    std::string toString() const;
    
    // Reset
    void reset();
    void resetToOrigin();
    
private:
    double x_, y_;
    CS* cs_;  // Pointer to coordinate system (not owned)
    
    // Helper methods
    Point2D transformByCS(const CS& cs) const;
};

#endif // POINT_2D_H

