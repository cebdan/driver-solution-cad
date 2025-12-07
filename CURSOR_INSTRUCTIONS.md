# Инструкции для Cursor: Переписать ядро CAD системы

## Задача
Полностью переписать архитектуру проекта. Удалить зависимость от xtd. Реализовать микрокерн на базе Solution с системой связей.

---

## 1. Удалить файлы

```
include/XTD.h
include/TerminalWindow.h
include/StartWindow.h
include/DataExchange.h
include/OpenGLRenderer.h
include/ConstructionHistory.h
include/Node.h
include/SolutionDocument.h
include/Document2D.h
include/2D_point.h
include/3D_point.h
include/close_app.h
include/macos_window_utils.h
include/imgui_macos_utils.h

src/XTD.cpp
src/TerminalWindow.cpp
src/StartWindow.cpp
src/DataExchange.cpp
src/OpenGLRenderer.cpp
src/ConstructionHistory.cpp
src/Node.cpp
src/SolutionDocument.cpp
src/Document2D.cpp
src/2D_point.cpp
src/3D_point.cpp
src/close_app.cpp
src/macos_window_utils.mm
src/imgui_macos_utils.mm
src/StartWindowWithMenu.cpp

examples/ (все файлы)
```

---

## 2. Новый include/Solution.h

```cpp
#ifndef SOLUTION_H
#define SOLUTION_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>

// Link types between solutions
enum class LinkType {
    None,       // Independent copy (duplicate)
    Partial,    // Base params sync, additions independent (copy)
    Full        // Everything syncs (propagate)
};

// Sync state
enum class SyncState {
    Active,     // Synchronization enabled
    Excluded    // Temporarily disabled
};

class Solution {
public:
    Solution();
    explicit Solution(const std::string& name);
    virtual ~Solution() = default;

    // Identity
    uint64_t id() const { return id_; }
    const std::string& name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    // Core operation - must be implemented by derived classes
    virtual void solve() = 0;

    // Linking operations
    virtual std::unique_ptr<Solution> duplicate() const = 0;  // Independent copy
    virtual std::unique_ptr<Solution> copy() = 0;             // Partial link
    virtual std::unique_ptr<Solution> propagate() = 0;        // Full link
    
    // Link management
    void exclude();                    // Pause sync
    void restore();                    // Resume sync
    void makeIndependent();            // Break link permanently
    LinkType linkType() const { return link_type_; }
    SyncState syncState() const { return sync_state_; }

    // Tool generation
    virtual std::unique_ptr<Solution> makeSimilar() const;

    // Graph connections (inputs/outputs)
    void addInput(Solution* input);
    void removeInput(Solution* input);
    const std::vector<Solution*>& inputs() const { return inputs_; }
    
    void addOutput(Solution* output);
    void removeOutput(Solution* output);
    const std::vector<Solution*>& outputs() const { return outputs_; }

    // Parent-child (linking hierarchy)
    Solution* parent() const { return parent_; }
    const std::vector<Solution*>& children() const { return children_; }

    // Change propagation
    void markDirty();
    bool isDirty() const { return dirty_; }
    void update();  // Recalculate if dirty

    // Notifications
    using ChangeCallback = std::function<void(Solution*)>;
    void onChanged(ChangeCallback callback);

protected:
    // For derived classes to set up parent-child relationship
    void setParent(Solution* parent);
    void addChild(Solution* child);
    void removeChild(Solution* child);
    
    // Notify all outputs about change
    void notifyOutputs();
    
    // Copy base parameters to target
    virtual void copyBaseTo(Solution* target) const;
    
    // Link type - accessible to derived classes
    LinkType link_type_ = LinkType::None;

private:
    static std::atomic<uint64_t> next_id_;
    
    uint64_t id_;
    std::string name_;
    
    // Graph
    std::vector<Solution*> inputs_;
    std::vector<Solution*> outputs_;
    
    // Linking
    Solution* parent_ = nullptr;
    std::vector<Solution*> children_;
    SyncState sync_state_ = SyncState::Active;
    
    // State
    bool dirty_ = true;
    std::vector<ChangeCallback> callbacks_;
};

#endif // SOLUTION_H
```

---

## 3. Новый src/Solution.cpp

```cpp
#include "Solution.h"
#include <algorithm>

std::atomic<uint64_t> Solution::next_id_{1};

Solution::Solution() 
    : id_(next_id_++), name_("Solution") {}

Solution::Solution(const std::string& name) 
    : id_(next_id_++), name_(name) {}

void Solution::exclude() {
    sync_state_ = SyncState::Excluded;
}

void Solution::restore() {
    sync_state_ = SyncState::Active;
    if (parent_ && link_type_ != LinkType::None) {
        markDirty();
    }
}

void Solution::makeIndependent() {
    if (parent_) {
        parent_->removeChild(this);
        parent_ = nullptr;
    }
    link_type_ = LinkType::None;
}

std::unique_ptr<Solution> Solution::makeSimilar() const {
    // Default implementation - override in derived classes
    // Returns a "recipe" that can be applied to other inputs
    return nullptr;
}

void Solution::addInput(Solution* input) {
    if (input && std::find(inputs_.begin(), inputs_.end(), input) == inputs_.end()) {
        inputs_.push_back(input);
        input->addOutput(this);
    }
}

void Solution::removeInput(Solution* input) {
    auto it = std::find(inputs_.begin(), inputs_.end(), input);
    if (it != inputs_.end()) {
        inputs_.erase(it);
        input->removeOutput(this);
    }
}

void Solution::addOutput(Solution* output) {
    if (output && std::find(outputs_.begin(), outputs_.end(), output) == outputs_.end()) {
        outputs_.push_back(output);
    }
}

void Solution::removeOutput(Solution* output) {
    auto it = std::find(outputs_.begin(), outputs_.end(), output);
    if (it != outputs_.end()) {
        outputs_.erase(it);
    }
}

void Solution::setParent(Solution* parent) {
    if (parent_ && parent_ != parent) {
        parent_->removeChild(this);
    }
    parent_ = parent;
    if (parent) {
        parent->addChild(this);
    }
}

void Solution::addChild(Solution* child) {
    if (child && std::find(children_.begin(), children_.end(), child) == children_.end()) {
        children_.push_back(child);
    }
}

void Solution::removeChild(Solution* child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
    }
}

void Solution::markDirty() {
    if (!dirty_) {
        dirty_ = true;
        notifyOutputs();
    }
}

void Solution::update() {
    if (dirty_) {
        // First update all inputs
        for (auto* input : inputs_) {
            input->update();
        }
        
        // Sync from parent if linked
        if (parent_ && sync_state_ == SyncState::Active) {
            if (link_type_ == LinkType::Full) {
                parent_->copyBaseTo(this);
            } else if (link_type_ == LinkType::Partial) {
                parent_->copyBaseTo(this);
            }
        }
        
        // Recalculate
        solve();
        dirty_ = false;
        
        // Notify callbacks
        for (auto& cb : callbacks_) {
            cb(this);
        }
    }
}

void Solution::notifyOutputs() {
    for (auto* output : outputs_) {
        output->markDirty();
    }
    // Also notify children
    for (auto* child : children_) {
        if (child->syncState() == SyncState::Active) {
            child->markDirty();
        }
    }
}

void Solution::onChanged(ChangeCallback callback) {
    callbacks_.push_back(std::move(callback));
}

void Solution::copyBaseTo(Solution* target) const {
    // Base implementation - override in derived classes
    target->name_ = name_;
}
```

---

## 4. Новый include/Value.h

```cpp
#ifndef VALUE_H
#define VALUE_H

#include "Solution.h"
#include <variant>
#include <string>

class Value : public Solution {
public:
    using DataType = std::variant<int, double, std::string, bool>;
    
    Value();
    explicit Value(int v);
    explicit Value(double v);
    explicit Value(const std::string& v);
    explicit Value(bool v);
    
    // Getters
    int asInt() const;
    double asDouble() const;
    std::string asString() const;
    bool asBool() const;
    
    // Setters
    void set(int v);
    void set(double v);
    void set(const std::string& v);
    void set(bool v);
    
    // Type check
    template<typename T>
    bool is() const { return std::holds_alternative<T>(data_); }
    
    // Operators
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    bool operator==(const Value& other) const;
    
    // Solution interface
    void solve() override;
    std::unique_ptr<Solution> duplicate() const override;
    std::unique_ptr<Solution> copy() override;
    std::unique_ptr<Solution> propagate() override;

protected:
    void copyBaseTo(Solution* target) const override;

private:
    DataType data_;
};

#endif // VALUE_H
```

---

## 5. Новый src/Value.cpp

```cpp
#include "Value.h"
#include <stdexcept>

Value::Value() : Solution("Value"), data_(0) {}
Value::Value(int v) : Solution("Value"), data_(v) {}
Value::Value(double v) : Solution("Value"), data_(v) {}
Value::Value(const std::string& v) : Solution("Value"), data_(v) {}
Value::Value(bool v) : Solution("Value"), data_(v) {}

int Value::asInt() const {
    if (auto* p = std::get_if<int>(&data_)) return *p;
    if (auto* p = std::get_if<double>(&data_)) return static_cast<int>(*p);
    if (auto* p = std::get_if<bool>(&data_)) return *p ? 1 : 0;
    throw std::runtime_error("Cannot convert to int");
}

double Value::asDouble() const {
    if (auto* p = std::get_if<double>(&data_)) return *p;
    if (auto* p = std::get_if<int>(&data_)) return static_cast<double>(*p);
    if (auto* p = std::get_if<bool>(&data_)) return *p ? 1.0 : 0.0;
    throw std::runtime_error("Cannot convert to double");
}

std::string Value::asString() const {
    if (auto* p = std::get_if<std::string>(&data_)) return *p;
    if (auto* p = std::get_if<int>(&data_)) return std::to_string(*p);
    if (auto* p = std::get_if<double>(&data_)) return std::to_string(*p);
    if (auto* p = std::get_if<bool>(&data_)) return *p ? "true" : "false";
    return "";
}

bool Value::asBool() const {
    if (auto* p = std::get_if<bool>(&data_)) return *p;
    if (auto* p = std::get_if<int>(&data_)) return *p != 0;
    if (auto* p = std::get_if<double>(&data_)) return *p != 0.0;
    if (auto* p = std::get_if<std::string>(&data_)) return !p->empty();
    return false;
}

void Value::set(int v) { data_ = v; markDirty(); }
void Value::set(double v) { data_ = v; markDirty(); }
void Value::set(const std::string& v) { data_ = v; markDirty(); }
void Value::set(bool v) { data_ = v; markDirty(); }

Value Value::operator+(const Value& other) const {
    if (is<int>() && other.is<int>()) return Value(asInt() + other.asInt());
    if (is<std::string>() || other.is<std::string>()) return Value(asString() + other.asString());
    return Value(asDouble() + other.asDouble());
}

Value Value::operator-(const Value& other) const {
    if (is<int>() && other.is<int>()) return Value(asInt() - other.asInt());
    return Value(asDouble() - other.asDouble());
}

Value Value::operator*(const Value& other) const {
    if (is<int>() && other.is<int>()) return Value(asInt() * other.asInt());
    return Value(asDouble() * other.asDouble());
}

Value Value::operator/(const Value& other) const {
    if (is<int>() && other.is<int>()) return Value(asInt() / other.asInt());
    return Value(asDouble() / other.asDouble());
}

bool Value::operator==(const Value& other) const {
    return data_ == other.data_;
}

void Value::solve() {
    // Value is a leaf node - nothing to compute
}

std::unique_ptr<Solution> Value::duplicate() const {
    auto v = std::make_unique<Value>();
    v->data_ = data_;
    v->setName(name());
    return v;
}

std::unique_ptr<Solution> Value::copy() {
    auto v = std::make_unique<Value>();
    v->data_ = data_;
    v->setName(name() + "_copy");
    v->setParent(const_cast<Value*>(this));
    v->link_type_ = LinkType::Partial;
    return v;
}

std::unique_ptr<Solution> Value::propagate() {
    auto v = std::make_unique<Value>();
    v->data_ = data_;
    v->setName(name() + "_propagate");
    v->setParent(const_cast<Value*>(this));
    v->link_type_ = LinkType::Full;
    return v;
}

void Value::copyBaseTo(Solution* target) const {
    Solution::copyBaseTo(target);
    if (auto* v = dynamic_cast<Value*>(target)) {
        v->data_ = data_;
    }
}
```

---

## 6. Новый include/CS.h

```cpp
#ifndef CS_H
#define CS_H

#include "Solution.h"

// Coordinate System: position (x,y,z) + rotation (a,b,c)
class CS : public Solution {
public:
    CS();
    CS(double x, double y, double z, double a = 0, double b = 0, double c = 0);
    
    // Position
    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }
    void setX(double v) { x_ = v; markDirty(); }
    void setY(double v) { y_ = v; markDirty(); }
    void setZ(double v) { z_ = v; markDirty(); }
    void setPosition(double x, double y, double z);
    
    // Rotation (radians)
    double a() const { return a_; }
    double b() const { return b_; }
    double c() const { return c_; }
    void setA(double v) { a_ = v; markDirty(); }
    void setB(double v) { b_ = v; markDirty(); }
    void setC(double v) { c_ = v; markDirty(); }
    void setRotation(double a, double b, double c);
    
    // Operations
    CS transform(const CS& other) const;
    CS inverse() const;
    double distanceTo(const CS& other) const;
    
    // Global CS (0,0,0,0,0,0)
    static CS global();
    
    // Solution interface
    void solve() override;
    std::unique_ptr<Solution> duplicate() const override;
    std::unique_ptr<Solution> copy() override;
    std::unique_ptr<Solution> propagate() override;

protected:
    void copyBaseTo(Solution* target) const override;

private:
    double x_, y_, z_;  // Position
    double a_, b_, c_;  // Rotation
};

#endif // CS_H
```

---

## 7. Новый src/CS.cpp

```cpp
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
```

---

## 8. Новый include/Point.h

```cpp
#ifndef POINT_H
#define POINT_H

#include "Solution.h"
#include "CS.h"

class Point : public Solution {
public:
    Point();
    Point(double x, double y, double z = 0);
    Point(double x, double y, double z, CS* cs);
    
    // Coordinates
    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }
    void setX(double v) { x_ = v; markDirty(); }
    void setY(double v) { y_ = v; markDirty(); }
    void setZ(double v) { z_ = v; markDirty(); }
    void set(double x, double y, double z = 0);
    
    // Coordinate system
    CS* cs() const { return cs_; }
    void setCS(CS* cs);
    
    // Transform to global coordinates
    Point toGlobal() const;
    
    // Operations
    double distanceTo(const Point& other) const;
    Point operator+(const Point& other) const;
    Point operator-(const Point& other) const;
    Point operator*(double scalar) const;
    double dot(const Point& other) const;
    Point cross(const Point& other) const;
    double length() const;
    Point normalized() const;
    
    // Solution interface
    void solve() override;
    std::unique_ptr<Solution> duplicate() const override;
    std::unique_ptr<Solution> copy() override;
    std::unique_ptr<Solution> propagate() override;

protected:
    void copyBaseTo(Solution* target) const override;

private:
    double x_, y_, z_;
    CS* cs_ = nullptr;  // Optional coordinate system
};

#endif // POINT_H
```

---

## 9. Новый src/Point.cpp

```cpp
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
```

---

## 10. Новый include/Line.h

```cpp
#ifndef LINE_H
#define LINE_H

#include "Solution.h"
#include "Point.h"

class Line : public Solution {
public:
    Line();
    Line(Point* start, Point* end);
    
    // Access points
    Point* start() const { return start_; }
    Point* end() const { return end_; }
    void setStart(Point* p);
    void setEnd(Point* p);
    
    // Computed properties
    double length() const { return length_; }
    Point direction() const { return direction_; }
    Point midpoint() const { return midpoint_; }
    
    // Solution interface
    void solve() override;
    std::unique_ptr<Solution> duplicate() const override;
    std::unique_ptr<Solution> copy() override;
    std::unique_ptr<Solution> propagate() override;

protected:
    void copyBaseTo(Solution* target) const override;

private:
    Point* start_ = nullptr;
    Point* end_ = nullptr;
    
    // Computed values (updated in solve())
    double length_ = 0;
    Point direction_;
    Point midpoint_;
};

#endif // LINE_H
```

---

## 11. Новый src/Line.cpp

```cpp
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
```

---

## 12. Новый CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(driver-solution-cad VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library
add_library(cad_core STATIC
    src/Solution.cpp
    src/Value.cpp
    src/CS.cpp
    src/Point.cpp
    src/Line.cpp
)

target_include_directories(cad_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Tests
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE cad_core)

# Example
add_executable(example_chain examples/example_chain.cpp)
target_link_libraries(example_chain PRIVATE cad_core)
```

---

## 13. Новый tests/test_core.cpp

```cpp
#include "Value.h"
#include "CS.h"
#include "Point.h"
#include "Line.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_value() {
    std::cout << "Testing Value..." << std::endl;
    
    Value v1(42);
    assert(v1.asInt() == 42);
    assert(v1.asDouble() == 42.0);
    
    Value v2(3.14);
    assert(std::abs(v2.asDouble() - 3.14) < 0.001);
    
    Value v3 = v1 + Value(8);
    assert(v3.asInt() == 50);
    
    // Test propagate
    auto v4 = v1.propagate();
    v1.set(100);
    v1.update();
    dynamic_cast<Value*>(v4.get())->update();
    assert(dynamic_cast<Value*>(v4.get())->asInt() == 100);
    
    std::cout << "  Value: OK" << std::endl;
}

void test_cs() {
    std::cout << "Testing CS..." << std::endl;
    
    CS cs1(10, 20, 30);
    assert(cs1.x() == 10);
    assert(cs1.y() == 20);
    assert(cs1.z() == 30);
    
    CS global = CS::global();
    assert(global.x() == 0);
    
    double dist = cs1.distanceTo(global);
    assert(std::abs(dist - std::sqrt(10*10 + 20*20 + 30*30)) < 0.001);
    
    std::cout << "  CS: OK" << std::endl;
}

void test_point() {
    std::cout << "Testing Point..." << std::endl;
    
    Point p1(3, 4, 0);
    assert(std::abs(p1.length() - 5.0) < 0.001);
    
    Point p2(1, 0, 0);
    Point p3(0, 1, 0);
    Point cross = p2.cross(p3);
    assert(std::abs(cross.z() - 1.0) < 0.001);
    
    // Test with CS
    CS cs(100, 0, 0);
    Point p4(10, 0, 0, &cs);
    Point global = p4.toGlobal();
    assert(std::abs(global.x() - 110) < 0.001);
    
    std::cout << "  Point: OK" << std::endl;
}

void test_line() {
    std::cout << "Testing Line..." << std::endl;
    
    Point p1(0, 0, 0);
    Point p2(10, 0, 0);
    Line line(&p1, &p2);
    line.update();
    
    assert(std::abs(line.length() - 10.0) < 0.001);
    assert(std::abs(line.midpoint().x() - 5.0) < 0.001);
    
    std::cout << "  Line: OK" << std::endl;
}

void test_chain() {
    std::cout << "Testing chain reaction..." << std::endl;
    
    Point p1(0, 0, 0);
    Point p2(10, 0, 0);
    Line line(&p1, &p2);
    
    line.update();
    assert(std::abs(line.length() - 10.0) < 0.001);
    
    // Change point - line should update
    p2.setX(20);
    line.update();
    assert(std::abs(line.length() - 20.0) < 0.001);
    
    std::cout << "  Chain: OK" << std::endl;
}

void test_linking() {
    std::cout << "Testing linking..." << std::endl;
    
    Point original(10, 20, 30);
    
    // Duplicate - independent
    auto dup = original.duplicate();
    original.setX(100);
    assert(dynamic_cast<Point*>(dup.get())->x() == 10);
    
    // Propagate - synced
    Point p2(5, 5, 5);
    auto prop = p2.propagate();
    p2.setX(50);
    p2.update();
    dynamic_cast<Point*>(prop.get())->update();
    assert(dynamic_cast<Point*>(prop.get())->x() == 50);
    
    // Exclude - pause sync
    dynamic_cast<Point*>(prop.get())->exclude();
    p2.setX(999);
    p2.update();
    dynamic_cast<Point*>(prop.get())->update();
    assert(dynamic_cast<Point*>(prop.get())->x() == 50);  // Still 50
    
    // Restore - resume sync
    dynamic_cast<Point*>(prop.get())->restore();
    dynamic_cast<Point*>(prop.get())->update();
    assert(dynamic_cast<Point*>(prop.get())->x() == 999);
    
    std::cout << "  Linking: OK" << std::endl;
}

int main() {
    std::cout << "=== CAD Core Tests ===" << std::endl;
    
    test_value();
    test_cs();
    test_point();
    test_line();
    test_chain();
    test_linking();
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
```

---

## 14. Новый examples/example_chain.cpp

```cpp
#include "Point.h"
#include "Line.h"
#include <iostream>

int main() {
    std::cout << "=== Chain Reaction Example ===" << std::endl;
    
    // Create triangle from points and lines
    Point p1(0, 0, 0);   p1.setName("P1");
    Point p2(10, 0, 0);  p2.setName("P2");
    Point p3(5, 8, 0);   p3.setName("P3");
    
    Line l1(&p1, &p2);   l1.setName("L1");
    Line l2(&p2, &p3);   l2.setName("L2");
    Line l3(&p3, &p1);   l3.setName("L3");
    
    // Update all
    l1.update();
    l2.update();
    l3.update();
    
    std::cout << "\nInitial triangle:" << std::endl;
    std::cout << "  L1 length: " << l1.length() << std::endl;
    std::cout << "  L2 length: " << l2.length() << std::endl;
    std::cout << "  L3 length: " << l3.length() << std::endl;
    
    // Move point - all connected lines update
    std::cout << "\nMoving P3 to (5, 16, 0)..." << std::endl;
    p3.setY(16);
    
    l1.update();
    l2.update();
    l3.update();
    
    std::cout << "  L1 length: " << l1.length() << " (unchanged)" << std::endl;
    std::cout << "  L2 length: " << l2.length() << " (changed)" << std::endl;
    std::cout << "  L3 length: " << l3.length() << " (changed)" << std::endl;
    
    // Test propagate
    std::cout << "\nCreating propagated copy of P1..." << std::endl;
    auto p1_copy = p1.propagate();
    Point* p1c = dynamic_cast<Point*>(p1_copy.get());
    
    std::cout << "  P1: (" << p1.x() << ", " << p1.y() << ")" << std::endl;
    std::cout << "  P1_copy: (" << p1c->x() << ", " << p1c->y() << ")" << std::endl;
    
    std::cout << "\nMoving P1 to (100, 100, 0)..." << std::endl;
    p1.set(100, 100, 0);
    p1.update();
    p1c->update();
    
    std::cout << "  P1: (" << p1.x() << ", " << p1.y() << ")" << std::endl;
    std::cout << "  P1_copy: (" << p1c->x() << ", " << p1c->y() << ") - synced!" << std::endl;
    
    return 0;
}
```

---

## 15. Новый PHILOSOPHY.md

```markdown
# Философия архитектуры CAD системы

## Принцип: "Everything is a Solution"

Каждый объект в системе наследует от `Solution`:
- Value (числа, строки, булевы)
- CS (координатные системы)
- Point (точки 2D/3D)
- Line (линии)
- Triangle, Circle, Extrude... (всё остальное)

## Система связей (Linking Model)

| Метод | Тип связи | Поведение |
|-------|-----------|-----------|
| `duplicate()` | None | Полностью независимая копия |
| `copy()` | Partial | Базовые параметры синхронны |
| `propagate()` | Full | Всё синхронизируется |
| `exclude()` | - | Временно отключить синхронизацию |
| `restore()` | - | Включить синхронизацию |
| `makeIndependent()` | - | Навсегда разорвать связь |

## Граф зависимостей

Solution имеет:
- `inputs_` — от кого зависит (Line зависит от Points)
- `outputs_` — кто зависит от него

Изменение автоматически распространяется через `markDirty()`.

## Цепная реакция

```
Point → Line → Triangle → Extrude
  ↓
изменился
  ↓
Line.markDirty() → Triangle.markDirty() → Extrude.markDirty()
  ↓
update() пересчитывает всё
```

## makeSimilar() — генератор инструментов

Извлекает "рецепт" операции как переиспользуемый Tool/Node.

## Правила

1. Каждый класс наследует Solution
2. solve() — чистая функция без побочных эффектов
3. Связи явные через copy/duplicate/propagate
4. GUI и рендеринг — отдельные слои, не в Solution
```

---

## 16. Структура после рефакторинга

```
driver-solution-cad/
├── CMakeLists.txt
├── PHILOSOPHY.md
├── README.md
├── include/
│   ├── Solution.h
│   ├── Value.h
│   ├── CS.h
│   ├── Point.h
│   └── Line.h
├── src/
│   ├── Solution.cpp
│   ├── Value.cpp
│   ├── CS.cpp
│   ├── Point.cpp
│   └── Line.cpp
├── tests/
│   └── test_core.cpp
└── examples/
    └── example_chain.cpp
```

---

## 17. Сборка и тестирование

```bash
cd driver-solution-cad
rm -rf build
mkdir build && cd build
cmake ..
make
./test_core
./example_chain
```

---

## Итого

1. **Удали** все старые файлы (xtd, GUI, терминал)
2. **Создай** новые файлы по шаблонам выше
3. **Собери** и запусти тесты
4. **Ядро готово** — GUI можно добавить потом отдельным слоем
