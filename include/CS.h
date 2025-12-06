#ifndef CS_H
#define CS_H

#include "Solution.h"
#include <cmath>
#include <string>

class CS : public Solution {
public:
    CS();
    CS(double x, double y, double z, double a = 0.0, double b = 0.0, double c = 0.0);
    virtual ~CS() = default;
    
    // Global coordinate system
    static CS GlobalCS();
    
    // Position coordinates (x, y, z)
    void setX(double x) { x_ = x; }
    void setY(double y) { y_ = y; }
    void setZ(double z) { z_ = z; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    
    // Rotation angles (a, b, c)
    void setA(double a) { a_ = a; }
    void setB(double b) { b_ = b; }
    void setC(double c) { c_ = c; }
    double getA() const { return a_; }
    double getB() const { return b_; }
    double getC() const { return c_; }
    
    // Set all coordinates at once
    void setPosition(double x, double y, double z);
    void setRotation(double a, double b, double c);
    void set(double x, double y, double z, double a = 0.0, double b = 0.0, double c = 0.0);
    
    // Get position and rotation as arrays
    void getPosition(double& x, double& y, double& z) const;
    void getRotation(double& a, double& b, double& c) const;
    void get(double& x, double& y, double& z, double& a, double& b, double& c) const;
    
    // Coordinate system operations
    CS transform(const CS& other) const;
    CS inverse() const;
    CS add(const CS& other) const;
    CS subtract(const CS& other) const;
    
    // Operators
    CS operator+(const CS& other) const;
    CS operator-(const CS& other) const;
    CS operator*(double scalar) const;
    bool operator==(const CS& other) const;
    bool operator!=(const CS& other) const;
    
    // Distance and angle calculations
    double distance(const CS& other) const;
    double distanceToOrigin() const;
    double angleDifference(const CS& other) const;
    
    // Transformation matrix (4x4 homogeneous transformation matrix)
    void getTransformationMatrix(double matrix[16]) const;
    void setFromTransformationMatrix(const double matrix[16]);
    
    // Reset to origin
    void reset();
    void resetPosition();
    void resetRotation();
    
    // Check if coordinate system is valid
    bool isValid() const;
    
    // String representation
    std::string toString() const;
    
    // Implementation of pure virtual methods from Solution
    virtual void solve() override;
    virtual void new_solution() override;
    virtual void delete_solution() override;
    virtual void copy() override;
    virtual void duplication() override;
    virtual void propagation() override;
    virtual void similar_make() override;
    
private:
    double x_, y_, z_;  // Position coordinates
    double a_, b_, c_;  // Rotation angles (in radians or degrees)
    
    // Helper methods for matrix operations
    void rotationMatrix(double matrix[9]) const;
    void eulerToMatrix(double a, double b, double c, double matrix[9]) const;
    void matrixToEuler(const double matrix[9], double& a, double& b, double& c) const;
};

#endif // CS_H

