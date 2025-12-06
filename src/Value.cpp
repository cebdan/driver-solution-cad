#include "../include/Value.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

Value::Value() : Solution(), type_(ValueType::UNDEFINED) {
    data_ = 0;
    setName("Value");
}

// Implementation of pure virtual methods from Solution
void Value::solve() {
    // Value solution implementation
}

void Value::new_solution() {
    clear();
}

void Value::delete_solution() {
    clear();
}

void Value::copy() {
    // Copy value data
}

void Value::duplication() {
    // Duplicate value
}

void Value::propagation() {
    // Propagate value
}

void Value::similar_make() {
    // Make similar value
}

void Value::setInt(int value) {
    type_ = ValueType::INTEGER;
    data_ = value;
}

void Value::setDouble(double value) {
    type_ = ValueType::DOUBLE;
    data_ = value;
}

void Value::setString(const std::string& value) {
    type_ = ValueType::STRING;
    data_ = value;
}

void Value::setBool(bool value) {
    type_ = ValueType::BOOLEAN;
    data_ = value;
}

void Value::setPointer(void* value) {
    type_ = ValueType::POINTER;
    data_ = value;
}

int Value::getInt() const {
    if (type_ == ValueType::INTEGER) {
        return std::get<int>(data_);
    } else if (type_ == ValueType::DOUBLE) {
        return static_cast<int>(std::get<double>(data_));
    } else if (type_ == ValueType::BOOLEAN) {
        return std::get<bool>(data_) ? 1 : 0;
    } else if (type_ == ValueType::STRING) {
        try {
            return std::stoi(std::get<std::string>(data_));
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

double Value::getDouble() const {
    if (type_ == ValueType::DOUBLE) {
        return std::get<double>(data_);
    } else if (type_ == ValueType::INTEGER) {
        return static_cast<double>(std::get<int>(data_));
    } else if (type_ == ValueType::BOOLEAN) {
        return std::get<bool>(data_) ? 1.0 : 0.0;
    } else if (type_ == ValueType::STRING) {
        try {
            return std::stod(std::get<std::string>(data_));
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

std::string Value::getString() const {
    if (type_ == ValueType::STRING) {
        return std::get<std::string>(data_);
    } else if (type_ == ValueType::INTEGER) {
        return std::to_string(std::get<int>(data_));
    } else if (type_ == ValueType::DOUBLE) {
        return std::to_string(std::get<double>(data_));
    } else if (type_ == ValueType::BOOLEAN) {
        return std::get<bool>(data_) ? "true" : "false";
    } else if (type_ == ValueType::POINTER) {
        std::ostringstream oss;
        oss << std::get<void*>(data_);
        return oss.str();
    }
    return "";
}

bool Value::getBool() const {
    if (type_ == ValueType::BOOLEAN) {
        return std::get<bool>(data_);
    } else if (type_ == ValueType::INTEGER) {
        return std::get<int>(data_) != 0;
    } else if (type_ == ValueType::DOUBLE) {
        return std::get<double>(data_) != 0.0;
    } else if (type_ == ValueType::STRING) {
        std::string str = std::get<std::string>(data_);
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str == "true" || str == "1" || str == "yes";
    }
    return false;
}

void* Value::getPointer() const {
    if (type_ == ValueType::POINTER) {
        return std::get<void*>(data_);
    }
    return nullptr;
}

std::string Value::toString() const {
    return getString();
}

bool Value::toBool() const {
    return getBool();
}

int Value::toInt() const {
    return getInt();
}

double Value::toDouble() const {
    return getDouble();
}

bool Value::isValid() const {
    return type_ != ValueType::UNDEFINED;
}

void Value::clear() {
    type_ = ValueType::UNDEFINED;
    data_ = 0;
}

bool Value::equals(const Value& other) const {
    if (type_ != other.type_) {
        // Try to compare by converting to same type
        if (type_ == ValueType::INTEGER && other.type_ == ValueType::DOUBLE) {
            return getInt() == other.getInt();
        }
        if (type_ == ValueType::DOUBLE && other.type_ == ValueType::INTEGER) {
            return getInt() == other.getInt();
        }
        return toString() == other.toString();
    }
    
    switch (type_) {
        case ValueType::INTEGER:
            return std::get<int>(data_) == std::get<int>(other.data_);
        case ValueType::DOUBLE:
            return std::get<double>(data_) == std::get<double>(other.data_);
        case ValueType::STRING:
            return std::get<std::string>(data_) == std::get<std::string>(other.data_);
        case ValueType::BOOLEAN:
            return std::get<bool>(data_) == std::get<bool>(other.data_);
        case ValueType::POINTER:
            return std::get<void*>(data_) == std::get<void*>(other.data_);
        default:
            return false;
    }
}

bool Value::operator==(const Value& other) const {
    return equals(other);
}

bool Value::operator!=(const Value& other) const {
    return !equals(other);
}

Value Value::operator+(const Value& other) const {
    Value result;
    if (type_ == ValueType::INTEGER && other.type_ == ValueType::INTEGER) {
        result.setInt(getInt() + other.getInt());
    } else if ((type_ == ValueType::DOUBLE || type_ == ValueType::INTEGER) &&
               (other.type_ == ValueType::DOUBLE || other.type_ == ValueType::INTEGER)) {
        result.setDouble(getDouble() + other.getDouble());
    } else if (type_ == ValueType::STRING || other.type_ == ValueType::STRING) {
        result.setString(toString() + other.toString());
    }
    return result;
}

Value Value::operator-(const Value& other) const {
    Value result;
    if (type_ == ValueType::INTEGER && other.type_ == ValueType::INTEGER) {
        result.setInt(getInt() - other.getInt());
    } else if ((type_ == ValueType::DOUBLE || type_ == ValueType::INTEGER) &&
               (other.type_ == ValueType::DOUBLE || other.type_ == ValueType::INTEGER)) {
        result.setDouble(getDouble() - other.getDouble());
    }
    return result;
}

Value Value::operator*(const Value& other) const {
    Value result;
    if (type_ == ValueType::INTEGER && other.type_ == ValueType::INTEGER) {
        result.setInt(getInt() * other.getInt());
    } else if ((type_ == ValueType::DOUBLE || type_ == ValueType::INTEGER) &&
               (other.type_ == ValueType::DOUBLE || other.type_ == ValueType::INTEGER)) {
        result.setDouble(getDouble() * other.getDouble());
    }
    return result;
}

Value Value::operator/(const Value& other) const {
    Value result;
    if (type_ == ValueType::INTEGER && other.type_ == ValueType::INTEGER) {
        int divisor = other.getInt();
        if (divisor != 0) {
            result.setInt(getInt() / divisor);
        }
    } else if ((type_ == ValueType::DOUBLE || type_ == ValueType::INTEGER) &&
               (other.type_ == ValueType::DOUBLE || other.type_ == ValueType::INTEGER)) {
        double divisor = other.getDouble();
        if (divisor != 0.0) {
            result.setDouble(getDouble() / divisor);
        }
    }
    return result;
}

void Value::convertTo(ValueType target_type) {
    if (type_ == target_type) {
        return;
    }
    
    switch (target_type) {
        case ValueType::INTEGER:
            setInt(getInt());
            break;
        case ValueType::DOUBLE:
            setDouble(getDouble());
            break;
        case ValueType::STRING:
            setString(toString());
            break;
        case ValueType::BOOLEAN:
            setBool(getBool());
            break;
        default:
            break;
    }
}

bool Value::canConvertTo(ValueType target_type) const {
    if (type_ == target_type) {
        return true;
    }
    
    switch (target_type) {
        case ValueType::INTEGER:
        case ValueType::DOUBLE:
            return type_ == ValueType::INTEGER || type_ == ValueType::DOUBLE || 
                   type_ == ValueType::BOOLEAN || type_ == ValueType::STRING;
        case ValueType::STRING:
            return true; // Can always convert to string
        case ValueType::BOOLEAN:
            return type_ != ValueType::POINTER && type_ != ValueType::UNDEFINED;
        default:
            return false;
    }
}

