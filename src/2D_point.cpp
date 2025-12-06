#include "../include/2D_point.h"
#include <sstream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Point2D::Point2D() : x_(0.0), y_(0.0), cs_(nullptr) {
}

Point2D::Point2D(double x, double y) : x_(x), y_(y), cs_(nullptr) {
}

Point2D::Point2D(double x, double y, CS* coordinate_system) 
    : x_(x), y_(y), cs_(coordinate_system) {
}

void Point2D::set(double x, double y) {
    x_ = x;
    y_ = y;
}

void Point2D::setCoordinateSystem(CS* cs) {
    cs_ = cs;
}

Point2D Point2D::add(const Point2D& other) const {
    Point2D result(x_ + other.x_, y_ + other.y_);
    if (cs_ && other.cs_ == cs_) {
        result.setCoordinateSystem(cs_);
    }
    return result;
}

Point2D Point2D::subtract(const Point2D& other) const {
    Point2D result(x_ - other.x_, y_ - other.y_);
    if (cs_ && other.cs_ == cs_) {
        result.setCoordinateSystem(cs_);
    }
    return result;
}

Point2D Point2D::multiply(double scalar) const {
    Point2D result(x_ * scalar, y_ * scalar);
    result.setCoordinateSystem(cs_);
    return result;
}

Point2D Point2D::divide(double scalar) const {
    if (std::abs(scalar) < 1e-9) {
        return Point2D(0.0, 0.0);
    }
    Point2D result(x_ / scalar, y_ / scalar);
    result.setCoordinateSystem(cs_);
    return result;
}

Point2D Point2D::operator+(const Point2D& other) const {
    return add(other);
}

Point2D Point2D::operator-(const Point2D& other) const {
    return subtract(other);
}

Point2D Point2D::operator*(double scalar) const {
    return multiply(scalar);
}

Point2D Point2D::operator/(double scalar) const {
    return divide(scalar);
}

bool Point2D::operator==(const Point2D& other) const {
    const double epsilon = 1e-9;
    return std::abs(x_ - other.x_) < epsilon && 
           std::abs(y_ - other.y_) < epsilon;
}

bool Point2D::operator!=(const Point2D& other) const {
    return !(*this == other);
}

double Point2D::distance(const Point2D& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    return std::sqrt(dx * dx + dy * dy);
}

double Point2D::distanceToOrigin() const {
    return std::sqrt(x_ * x_ + y_ * y_);
}

double Point2D::distanceSquared(const Point2D& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    return dx * dx + dy * dy;
}

double Point2D::dot(const Point2D& other) const {
    return x_ * other.x_ + y_ * other.y_;
}

double Point2D::cross(const Point2D& other) const {
    return x_ * other.y_ - y_ * other.x_;
}

double Point2D::length() const {
    return distanceToOrigin();
}

double Point2D::lengthSquared() const {
    return x_ * x_ + y_ * y_;
}

Point2D Point2D::normalize() const {
    double len = length();
    if (len < 1e-9) {
        return Point2D(0.0, 0.0);
    }
    return Point2D(x_ / len, y_ / len);
}

double Point2D::angle(const Point2D& other) const {
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

double Point2D::angleToOrigin() const {
    if (std::abs(x_) < 1e-9 && std::abs(y_) < 1e-9) {
        return 0.0;
    }
    return std::atan2(y_, x_);
}

Point2D Point2D::transform(const CS& target_cs) const {
    return transformByCS(target_cs);
}

Point2D Point2D::transformToGlobal() const {
    if (!cs_) {
        return *this;
    }
    
    // Transform point from local CS to global CS
    // Get CS transformation
    double cs_x = cs_->getX();
    double cs_y = cs_->getY();
    double cs_z = cs_->getZ();
    double cs_a = cs_->getA();
    double cs_b = cs_->getB();
    double cs_c = cs_->getC();
    
    // Simple 2D transformation (ignoring z and rotation for now)
    // In full implementation, would use rotation matrix
    double global_x = cs_x + x_;
    double global_y = cs_y + y_;
    
    Point2D result(global_x, global_y);
    // Result is in global CS, so no CS pointer
    return result;
}

Point2D Point2D::rotate(double angle) const {
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    double new_x = x_ * cos_a - y_ * sin_a;
    double new_y = x_ * sin_a + y_ * cos_a;
    Point2D result(new_x, new_y);
    result.setCoordinateSystem(cs_);
    return result;
}

Point2D Point2D::translate(double dx, double dy) const {
    Point2D result(x_ + dx, y_ + dy);
    result.setCoordinateSystem(cs_);
    return result;
}

Point2D Point2D::scale(double sx, double sy) const {
    Point2D result(x_ * sx, y_ * sy);
    result.setCoordinateSystem(cs_);
    return result;
}

void Point2D::to3D(double& x, double& y, double& z) const {
    x = x_;
    y = y_;
    z = 0.0;
    
    // If point has coordinate system, transform to global 3D
    if (cs_) {
        x += cs_->getX();
        y += cs_->getY();
        z = cs_->getZ();
    }
}

bool Point2D::isValid() const {
    return std::isfinite(x_) && std::isfinite(y_);
}

bool Point2D::isZero() const {
    const double epsilon = 1e-9;
    return std::abs(x_) < epsilon && std::abs(y_) < epsilon;
}

std::string Point2D::toString() const {
    std::ostringstream oss;
    oss << "Point2D(" << x_ << ", " << y_;
    if (cs_) {
        oss << ", CS: " << cs_->getName();
    }
    oss << ")";
    return oss.str();
}

void Point2D::reset() {
    x_ = 0.0;
    y_ = 0.0;
    cs_ = nullptr;
}

void Point2D::resetToOrigin() {
    x_ = 0.0;
    y_ = 0.0;
}

Point2D Point2D::transformByCS(const CS& cs) const {
    // Transform point coordinates using CS transformation
    double cs_x = cs.getX();
    double cs_y = cs.getY();
    
    // Simple translation (full implementation would include rotation)
    double new_x = cs_x + x_;
    double new_y = cs_y + y_;
    
    Point2D result(new_x, new_y);
    return result;
}

