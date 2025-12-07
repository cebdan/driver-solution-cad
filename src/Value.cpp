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
