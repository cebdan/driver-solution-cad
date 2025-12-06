#include "../include/3D_point.h"
#include <sstream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Point3D::Point3D() : x_(0.0), y_(0.0), z_(0.0), cs_(nullptr) {
}

Point3D::Point3D(double x, double y, double z) : x_(x), y_(y), z_(z), cs_(nullptr) {
}

Point3D::Point3D(double x, double y, double z, CS* coordinate_system) 
    : x_(x), y_(y), z_(z), cs_(coordinate_system) {
}

void Point3D::set(double x, double y, double z) {
    x_ = x;
    y_ = y;
    z_ = z;
}

void Point3D::setCoordinateSystem(CS* cs) {
    cs_ = cs;
}

Point3D Point3D::add(const Point3D& other) const {
    Point3D result(x_ + other.x_, y_ + other.y_, z_ + other.z_);
    if (cs_ && other.cs_ == cs_) {
        result.setCoordinateSystem(cs_);
    }
    return result;
}

Point3D Point3D::subtract(const Point3D& other) const {
    Point3D result(x_ - other.x_, y_ - other.y_, z_ - other.z_);
    if (cs_ && other.cs_ == cs_) {
        result.setCoordinateSystem(cs_);
    }
    return result;
}

Point3D Point3D::multiply(double scalar) const {
    Point3D result(x_ * scalar, y_ * scalar, z_ * scalar);
    result.setCoordinateSystem(cs_);
    return result;
}

Point3D Point3D::divide(double scalar) const {
    if (std::abs(scalar) < 1e-9) {
        return Point3D(0.0, 0.0, 0.0);
    }
    Point3D result(x_ / scalar, y_ / scalar, z_ / scalar);
    result.setCoordinateSystem(cs_);
    return result;
}

Point3D Point3D::operator+(const Point3D& other) const {
    return add(other);
}

Point3D Point3D::operator-(const Point3D& other) const {
    return subtract(other);
}

Point3D Point3D::operator*(double scalar) const {
    return multiply(scalar);
}

Point3D Point3D::operator/(double scalar) const {
    return divide(scalar);
}

bool Point3D::operator==(const Point3D& other) const {
    const double epsilon = 1e-9;
    return std::abs(x_ - other.x_) < epsilon && 
           std::abs(y_ - other.y_) < epsilon &&
           std::abs(z_ - other.z_) < epsilon;
}

bool Point3D::operator!=(const Point3D& other) const {
    return !(*this == other);
}

double Point3D::distance(const Point3D& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    double dz = z_ - other.z_;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

double Point3D::distanceToOrigin() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

double Point3D::distanceSquared(const Point3D& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    double dz = z_ - other.z_;
    return dx * dx + dy * dy + dz * dz;
}

double Point3D::dot(const Point3D& other) const {
    return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
}

Point3D Point3D::cross(const Point3D& other) const {
    return Point3D(
        y_ * other.z_ - z_ * other.y_,
        z_ * other.x_ - x_ * other.z_,
        x_ * other.y_ - y_ * other.x_
    );
}

double Point3D::length() const {
    return distanceToOrigin();
}

double Point3D::lengthSquared() const {
    return x_ * x_ + y_ * y_ + z_ * z_;
}

Point3D Point3D::normalize() const {
    double len = length();
    if (len < 1e-9) {
        return Point3D(0.0, 0.0, 0.0);
    }
    return Point3D(x_ / len, y_ / len, z_ / len);
}

double Point3D::angle(const Point3D& other) const {
    double dot_product = dot(other);
    double len1 = length();
    double len2 = other.length();
    
    if (len1 < 1e-9 || len2 < 1e-9) {
        return 0.0;
    }
    
    double cos_angle = dot_product / (len1 * len2);
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle)); // Clamp to [-1, 1]
    return std::acos(cos_angle);
}

Point3D Point3D::transform(const CS& target_cs) const {
    return transformByCS(target_cs);
}

Point3D Point3D::transformToGlobal() const {
    if (!cs_) {
        return *this;
    }
    
    // Transform point from local CS to global CS
    double cs_x = cs_->getX();
    double cs_y = cs_->getY();
    double cs_z = cs_->getZ();
    double cs_a = cs_->getA();
    double cs_b = cs_->getB();
    double cs_c = cs_->getC();
    
    // Get rotation matrix from CS
    double rot_matrix[9];
    double temp_x = x_, temp_y = y_, temp_z = z_;
    
    // Apply rotation
    double matrix[16];
    cs_->getTransformationMatrix(matrix);
    
    // Extract rotation part
    double rot[9] = {
        matrix[0], matrix[1], matrix[2],
        matrix[4], matrix[5], matrix[6],
        matrix[8], matrix[9], matrix[10]
    };
    
    applyRotationMatrix(rot, temp_x, temp_y, temp_z);
    
    // Apply translation
    double global_x = cs_x + temp_x;
    double global_y = cs_y + temp_y;
    double global_z = cs_z + temp_z;
    
    Point3D result(global_x, global_y, global_z);
    // Result is in global CS, so no CS pointer
    return result;
}

Point3D Point3D::rotate(double angle_x, double angle_y, double angle_z) const {
    // Rotate around X axis
    double cos_x = std::cos(angle_x);
    double sin_x = std::sin(angle_x);
    double y1 = y_ * cos_x - z_ * sin_x;
    double z1 = y_ * sin_x + z_ * cos_x;
    
    // Rotate around Y axis
    double cos_y = std::cos(angle_y);
    double sin_y = std::sin(angle_y);
    double x2 = x_ * cos_y + z1 * sin_y;
    double z2 = -x_ * sin_y + z1 * cos_y;
    
    // Rotate around Z axis
    double cos_z = std::cos(angle_z);
    double sin_z = std::sin(angle_z);
    double x3 = x2 * cos_z - y1 * sin_z;
    double y3 = x2 * sin_z + y1 * cos_z;
    
    Point3D result(x3, y3, z2);
    result.setCoordinateSystem(cs_);
    return result;
}

Point3D Point3D::translate(double dx, double dy, double dz) const {
    Point3D result(x_ + dx, y_ + dy, z_ + dz);
    result.setCoordinateSystem(cs_);
    return result;
}

Point3D Point3D::scale(double sx, double sy, double sz) const {
    Point3D result(x_ * sx, y_ * sy, z_ * sz);
    result.setCoordinateSystem(cs_);
    return result;
}

void Point3D::projectTo2D(double& x, double& y, int axis) const {
    switch (axis) {
        case 0: // Project to YZ plane (remove X)
            x = y_;
            y = z_;
            break;
        case 1: // Project to XZ plane (remove Y)
            x = x_;
            y = z_;
            break;
        case 2: // Project to XY plane (remove Z) - default
        default:
            x = x_;
            y = y_;
            break;
    }
}

Point3D Point3D::from2D(double x, double y, double z) {
    return Point3D(x, y, z);
}

bool Point3D::isValid() const {
    return std::isfinite(x_) && std::isfinite(y_) && std::isfinite(z_);
}

bool Point3D::isZero() const {
    const double epsilon = 1e-9;
    return std::abs(x_) < epsilon && 
           std::abs(y_) < epsilon && 
           std::abs(z_) < epsilon;
}

std::string Point3D::toString() const {
    std::ostringstream oss;
    oss << "Point3D(" << x_ << ", " << y_ << ", " << z_;
    if (cs_) {
        oss << ", CS: " << cs_->getName();
    }
    oss << ")";
    return oss.str();
}

void Point3D::reset() {
    x_ = 0.0;
    y_ = 0.0;
    z_ = 0.0;
    cs_ = nullptr;
}

void Point3D::resetToOrigin() {
    x_ = 0.0;
    y_ = 0.0;
    z_ = 0.0;
}

Point3D Point3D::transformByCS(const CS& cs) const {
    // Transform point coordinates using CS transformation
    double cs_x = cs.getX();
    double cs_y = cs.getY();
    double cs_z = cs.getZ();
    
    // Get transformation matrix
    double matrix[16];
    cs.getTransformationMatrix(matrix);
    
    // Extract rotation part
    double rot[9] = {
        matrix[0], matrix[1], matrix[2],
        matrix[4], matrix[5], matrix[6],
        matrix[8], matrix[9], matrix[10]
    };
    
    double new_x = x_, new_y = y_, new_z = z_;
    applyRotationMatrix(rot, new_x, new_y, new_z);
    
    // Apply translation
    double result_x = cs_x + new_x;
    double result_y = cs_y + new_y;
    double result_z = cs_z + new_z;
    
    Point3D result(result_x, result_y, result_z);
    return result;
}

void Point3D::applyRotationMatrix(const double matrix[9], double& x, double& y, double& z) const {
    double new_x = matrix[0] * x + matrix[1] * y + matrix[2] * z;
    double new_y = matrix[3] * x + matrix[4] * y + matrix[5] * z;
    double new_z = matrix[6] * x + matrix[7] * y + matrix[8] * z;
    x = new_x;
    y = new_y;
    z = new_z;
}

