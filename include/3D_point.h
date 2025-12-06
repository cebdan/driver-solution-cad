#ifndef POINT_3D_H
#define POINT_3D_H

#include "CS.h"
#include <memory>
#include <cmath>
#include <string>

class Point3D {
public:
    Point3D();
    Point3D(double x, double y, double z);
    Point3D(double x, double y, double z, CS* coordinate_system);
    virtual ~Point3D() = default;
    
    // Coordinate access
    void setX(double x) { x_ = x; }
    void setY(double y) { y_ = y; }
    void setZ(double z) { z_ = z; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    void set(double x, double y, double z);
    
    // Coordinate system management
    void setCoordinateSystem(CS* cs);
    CS* getCoordinateSystem() const { return cs_; }
    bool hasCoordinateSystem() const { return cs_ != nullptr; }
    
    // Point operations
    Point3D add(const Point3D& other) const;
    Point3D subtract(const Point3D& other) const;
    Point3D multiply(double scalar) const;
    Point3D divide(double scalar) const;
    
    // Operators
    Point3D operator+(const Point3D& other) const;
    Point3D operator-(const Point3D& other) const;
    Point3D operator*(double scalar) const;
    Point3D operator/(double scalar) const;
    bool operator==(const Point3D& other) const;
    bool operator!=(const Point3D& other) const;
    
    // Distance calculations
    double distance(const Point3D& other) const;
    double distanceToOrigin() const;
    double distanceSquared(const Point3D& other) const;
    
    // Geometric operations
    double dot(const Point3D& other) const;
    Point3D cross(const Point3D& other) const;
    double length() const;
    double lengthSquared() const;
    Point3D normalize() const;
    
    // Angle calculations
    double angle(const Point3D& other) const;
    
    // Transformation
    Point3D transform(const CS& target_cs) const;
    Point3D transformToGlobal() const;
    Point3D rotate(double angle_x, double angle_y, double angle_z) const;
    Point3D translate(double dx, double dy, double dz) const;
    Point3D scale(double sx, double sy, double sz) const;
    
    // Projection to 2D
    void projectTo2D(double& x, double& y, int axis = 2) const; // axis: 0=X, 1=Y, 2=Z
    
    // Conversion from 2D (using z=0 or specified z)
    static Point3D from2D(double x, double y, double z = 0.0);
    
    // Validation
    bool isValid() const;
    bool isZero() const;
    
    // String representation
    std::string toString() const;
    
    // Reset
    void reset();
    void resetToOrigin();
    
private:
    double x_, y_, z_;
    CS* cs_;  // Pointer to coordinate system (not owned)
    
    // Helper methods
    Point3D transformByCS(const CS& cs) const;
    void applyRotationMatrix(const double matrix[9], double& x, double& y, double& z) const;
};

#endif // POINT_3D_H

