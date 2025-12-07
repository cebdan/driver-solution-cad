#include "Line.h"
#include <cmath>

Line::Line() : Solution("Line") {}

Line::Line(Point* start, Point* end) : Solution("Line") {
    setStart(start);
    setEnd(end);
}

void Line::setStart(Point* p) {
    if (start_ != p) {
        if (start_) removeInput(start_);
        start_ = p;
        if (start_) addInput(start_);
        markDirty();
    }
}

void Line::setEnd(Point* p) {
    if (end_ != p) {
        if (end_) removeInput(end_);
        end_ = p;
        if (end_) addInput(end_);
        markDirty();
    }
}

void Line::solve() {
    if (!start_ || !end_) {
        length_ = 0;
        direction_ = Point();
        midpoint_ = Point();
        return;
    }
    
    // Calculate length
    length_ = start_->distanceTo(*end_);
    
    // Calculate direction (normalized)
    Point delta = *end_ - *start_;
    if (length_ > 1e-10) {
        direction_ = delta * (1.0 / length_);
    } else {
        direction_ = Point();
    }
    
    // Calculate midpoint
    midpoint_ = Point(
        (start_->x() + end_->x()) / 2,
        (start_->y() + end_->y()) / 2,
        (start_->z() + end_->z()) / 2
    );
}

std::unique_ptr<Solution> Line::duplicate() const {
    // Note: duplicating a line doesn't duplicate its points
    auto l = std::make_unique<Line>();
    l->setName(name());
    return l;
}

std::unique_ptr<Solution> Line::copy() {
    auto l = std::make_unique<Line>();
    l->setName(name() + "_copy");
    l->setParent(const_cast<Line*>(this));
    l->link_type_ = LinkType::Partial;
    return l;
}

std::unique_ptr<Solution> Line::propagate() {
    auto l = std::make_unique<Line>();
    l->setName(name() + "_propagate");
    l->setParent(const_cast<Line*>(this));
    l->link_type_ = LinkType::Full;
    return l;
}

void Line::copyBaseTo(Solution* target) const {
    Solution::copyBaseTo(target);
    // Line's base is its connection to points
    // Actual point data comes from the points themselves
}

