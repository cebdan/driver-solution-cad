#include "../include/CS.h"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CS::CS() : Solution(), x_(0.0), y_(0.0), z_(0.0), a_(0.0), b_(0.0), c_(0.0) {
    setName("CS");
}

CS::CS(double x, double y, double z, double a, double b, double c)
    : Solution(), x_(x), y_(y), z_(z), a_(a), b_(b), c_(c) {
    setName("CS");
}

CS CS::GlobalCS() {
    CS global(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    global.setName("Global CS");
    return global;
}

void CS::setPosition(double x, double y, double z) {
    x_ = x;
    y_ = y;
    z_ = z;
}

void CS::setRotation(double a, double b, double c) {
    a_ = a;
    b_ = b;
    c_ = c;
}

void CS::set(double x, double y, double z, double a, double b, double c) {
    x_ = x;
    y_ = y;
    z_ = z;
    a_ = a;
    b_ = b;
    c_ = c;
}

void CS::getPosition(double& x, double& y, double& z) const {
    x = x_;
    y = y_;
    z = z_;
}

void CS::getRotation(double& a, double& b, double& c) const {
    a = a_;
    b = b_;
    c = c_;
}

void CS::get(double& x, double& y, double& z, double& a, double& b, double& c) const {
    x = x_;
    y = y_;
    z = z_;
    a = a_;
    b = b_;
    c = c_;
}

CS CS::transform(const CS& other) const {
    CS result;
    
    // Transform position
    double tx = x_ + other.x_;
    double ty = y_ + other.y_;
    double tz = z_ + other.z_;
    
    // Transform rotation (simple addition for now, can be improved with matrix multiplication)
    double ta = a_ + other.a_;
    double tb = b_ + other.b_;
    double tc = c_ + other.c_;
    
    result.set(tx, ty, tz, ta, tb, tc);
    return result;
}

CS CS::inverse() const {
    CS result;
    result.set(-x_, -y_, -z_, -a_, -b_, -c_);
    return result;
}

CS CS::add(const CS& other) const {
    return CS(x_ + other.x_, y_ + other.y_, z_ + other.z_,
              a_ + other.a_, b_ + other.b_, c_ + other.c_);
}

CS CS::subtract(const CS& other) const {
    return CS(x_ - other.x_, y_ - other.y_, z_ - other.z_,
              a_ - other.a_, b_ - other.b_, c_ - other.c_);
}

CS CS::operator+(const CS& other) const {
    return add(other);
}

CS CS::operator-(const CS& other) const {
    return subtract(other);
}

CS CS::operator*(double scalar) const {
    return CS(x_ * scalar, y_ * scalar, z_ * scalar,
              a_ * scalar, b_ * scalar, c_ * scalar);
}

bool CS::operator==(const CS& other) const {
    const double epsilon = 1e-9;
    return std::abs(x_ - other.x_) < epsilon &&
           std::abs(y_ - other.y_) < epsilon &&
           std::abs(z_ - other.z_) < epsilon &&
           std::abs(a_ - other.a_) < epsilon &&
           std::abs(b_ - other.b_) < epsilon &&
           std::abs(c_ - other.c_) < epsilon;
}

bool CS::operator!=(const CS& other) const {
    return !(*this == other);
}

double CS::distance(const CS& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    double dz = z_ - other.z_;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

double CS::distanceToOrigin() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

double CS::angleDifference(const CS& other) const {
    double da = a_ - other.a_;
    double db = b_ - other.b_;
    double dc = c_ - other.c_;
    return std::sqrt(da * da + db * db + dc * dc);
}

void CS::getTransformationMatrix(double matrix[16]) const {
    // Initialize as identity matrix
    std::fill(matrix, matrix + 16, 0.0);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0;
    
    // Set translation
    matrix[12] = x_;
    matrix[13] = y_;
    matrix[14] = z_;
    
    // Calculate rotation matrix
    double rot[9];
    rotationMatrix(rot);
    
    // Copy rotation to transformation matrix
    matrix[0] = rot[0]; matrix[1] = rot[1]; matrix[2] = rot[2];
    matrix[4] = rot[3]; matrix[5] = rot[4]; matrix[6] = rot[5];
    matrix[8] = rot[6]; matrix[9] = rot[7]; matrix[10] = rot[8];
}

void CS::setFromTransformationMatrix(const double matrix[16]) {
    // Extract translation
    x_ = matrix[12];
    y_ = matrix[13];
    z_ = matrix[14];
    
    // Extract rotation matrix
    double rot[9] = {
        matrix[0], matrix[1], matrix[2],
        matrix[4], matrix[5], matrix[6],
        matrix[8], matrix[9], matrix[10]
    };
    
    // Convert to Euler angles
    matrixToEuler(rot, a_, b_, c_);
}

void CS::reset() {
    x_ = y_ = z_ = 0.0;
    a_ = b_ = c_ = 0.0;
}

void CS::resetPosition() {
    x_ = y_ = z_ = 0.0;
}

void CS::resetRotation() {
    a_ = b_ = c_ = 0.0;
}

bool CS::isValid() const {
    // Check for NaN or infinity
    return std::isfinite(x_) && std::isfinite(y_) && std::isfinite(z_) &&
           std::isfinite(a_) && std::isfinite(b_) && std::isfinite(c_);
}

std::string CS::toString() const {
    std::ostringstream oss;
    oss << "CS(" << x_ << ", " << y_ << ", " << z_ << ", "
        << a_ << ", " << b_ << ", " << c_ << ")";
    return oss.str();
}

void CS::solve() {
    // CS solution implementation
}

void CS::new_solution() {
    reset();
}

void CS::delete_solution() {
    reset();
}

void CS::copy() {
    // Copy CS data
}

void CS::duplication() {
    // Duplicate CS
}

void CS::propagation() {
    // Propagate CS
}

void CS::similar_make() {
    // Make similar CS
}

void CS::rotationMatrix(double matrix[9]) const {
    eulerToMatrix(a_, b_, c_, matrix);
}

void CS::eulerToMatrix(double a, double b, double c, double matrix[9]) const {
    // ZYX Euler angles (yaw, pitch, roll)
    double ca = std::cos(a);
    double sa = std::sin(a);
    double cb = std::cos(b);
    double sb = std::sin(b);
    double cc = std::cos(c);
    double sc = std::sin(c);
    
    matrix[0] = ca * cb;
    matrix[1] = ca * sb * sc - sa * cc;
    matrix[2] = ca * sb * cc + sa * sc;
    matrix[3] = sa * cb;
    matrix[4] = sa * sb * sc + ca * cc;
    matrix[5] = sa * sb * cc - ca * sc;
    matrix[6] = -sb;
    matrix[7] = cb * sc;
    matrix[8] = cb * cc;
}

void CS::matrixToEuler(const double matrix[9], double& a, double& b, double& c) const {
    // Extract Euler angles from rotation matrix (ZYX convention)
    b = std::asin(-matrix[6]);
    
    if (std::abs(std::cos(b)) > 1e-6) {
        a = std::atan2(matrix[3], matrix[0]);
        c = std::atan2(matrix[7], matrix[8]);
    } else {
        // Gimbal lock case
        a = std::atan2(-matrix[1], matrix[4]);
        c = 0.0;
    }
}

