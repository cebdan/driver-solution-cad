#include "Point.h"
#include <cmath>

Point::Point() : Solution("Point"), x_(0), y_(0), z_(0) {}

Point::Point(double x, double y, double z) 
    : Solution("Point"), x_(x), y_(y), z_(z) {}

Point::Point(double x, double y, double z, CS* cs) 
    : Solution("Point"), x_(x), y_(y), z_(z), cs_(cs) {
    if (cs) addInput(cs);
}

void Point::set(double x, double y, double z) {
    x_ = x; y_ = y; z_ = z;
    markDirty();
}

void Point::setCS(CS* cs) {
    if (cs_ != cs) {
        if (cs_) removeInput(cs_);
        cs_ = cs;
        if (cs_) addInput(cs_);
        markDirty();
    }
}

Point Point::toGlobal() const {
    if (!cs_) return *this;
    return Point(x_ + cs_->x(), y_ + cs_->y(), z_ + cs_->z());
}

double Point::distanceTo(const Point& other) const {
    Point a = toGlobal();
    Point b = other.toGlobal();
    double dx = a.x_ - b.x_;
    double dy = a.y_ - b.y_;
    double dz = a.z_ - b.z_;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

Point Point::operator+(const Point& other) const {
    return Point(x_ + other.x_, y_ + other.y_, z_ + other.z_);
}

Point Point::operator-(const Point& other) const {
    return Point(x_ - other.x_, y_ - other.y_, z_ - other.z_);
}

Point Point::operator*(double scalar) const {
    return Point(x_ * scalar, y_ * scalar, z_ * scalar);
}

double Point::dot(const Point& other) const {
    return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
}

Point Point::cross(const Point& other) const {
    return Point(
        y_ * other.z_ - z_ * other.y_,
        z_ * other.x_ - x_ * other.z_,
        x_ * other.y_ - y_ * other.x_
    );
}

double Point::length() const {
    return std::sqrt(x_*x_ + y_*y_ + z_*z_);
}

Point Point::normalized() const {
    double len = length();
    if (len < 1e-10) return Point();
    return Point(x_/len, y_/len, z_/len);
}

void Point::solve() {
    // If has CS input, coordinates are relative to it
    // Nothing else to compute for a point
}

std::unique_ptr<Solution> Point::duplicate() const {
    auto p = std::make_unique<Point>(x_, y_, z_);
    p->setName(name());
    // Note: CS is not duplicated, just referenced
    return p;
}

std::unique_ptr<Solution> Point::copy() {
    auto p = std::make_unique<Point>(x_, y_, z_);
    p->setName(name() + "_copy");
    p->setParent(const_cast<Point*>(this));
    p->link_type_ = LinkType::Partial;
    return p;
}

std::unique_ptr<Solution> Point::propagate() {
    auto p = std::make_unique<Point>(x_, y_, z_);
    p->setName(name() + "_propagate");
    p->setParent(const_cast<Point*>(this));
    p->link_type_ = LinkType::Full;
    return p;
}

void Point::copyBaseTo(Solution* target) const {
    Solution::copyBaseTo(target);
    if (auto* p = dynamic_cast<Point*>(target)) {
        p->x_ = x_; p->y_ = y_; p->z_ = z_;
    }
}

