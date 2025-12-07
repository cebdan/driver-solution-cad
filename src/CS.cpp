#include "CS.h"
#include <cmath>

CS::CS() : Solution("CS"), x_(0), y_(0), z_(0), a_(0), b_(0), c_(0) {}

CS::CS(double x, double y, double z, double a, double b, double c)
    : Solution("CS"), x_(x), y_(y), z_(z), a_(a), b_(b), c_(c) {}

void CS::setPosition(double x, double y, double z) {
    x_ = x; y_ = y; z_ = z;
    markDirty();
}

void CS::setRotation(double a, double b, double c) {
    a_ = a; b_ = b; c_ = c;
    markDirty();
}

CS CS::transform(const CS& other) const {
    // Simple transform: add positions and rotations
    return CS(x_ + other.x_, y_ + other.y_, z_ + other.z_,
              a_ + other.a_, b_ + other.b_, c_ + other.c_);
}

CS CS::inverse() const {
    return CS(-x_, -y_, -z_, -a_, -b_, -c_);
}

double CS::distanceTo(const CS& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    double dz = z_ - other.z_;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

CS CS::global() {
    CS cs;
    cs.setName("GlobalCS");
    return cs;
}

void CS::solve() {
    // CS is typically a leaf - nothing to compute
}

std::unique_ptr<Solution> CS::duplicate() const {
    auto cs = std::make_unique<CS>(x_, y_, z_, a_, b_, c_);
    cs->setName(name());
    return cs;
}

std::unique_ptr<Solution> CS::copy() {
    auto cs = std::make_unique<CS>(x_, y_, z_, a_, b_, c_);
    cs->setName(name() + "_copy");
    cs->setParent(const_cast<CS*>(this));
    cs->link_type_ = LinkType::Partial;
    return cs;
}

std::unique_ptr<Solution> CS::propagate() {
    auto cs = std::make_unique<CS>(x_, y_, z_, a_, b_, c_);
    cs->setName(name() + "_propagate");
    cs->setParent(const_cast<CS*>(this));
    cs->link_type_ = LinkType::Full;
    return cs;
}

void CS::copyBaseTo(Solution* target) const {
    Solution::copyBaseTo(target);
    if (auto* cs = dynamic_cast<CS*>(target)) {
        cs->x_ = x_; cs->y_ = y_; cs->z_ = z_;
        cs->a_ = a_; cs->b_ = b_; cs->c_ = c_;
    }
}
