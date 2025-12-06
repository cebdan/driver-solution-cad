#ifndef VALUE_H
#define VALUE_H

#include "Solution.h"
#include <string>
#include <variant>
#include <memory>

class Value : public Solution {
public:
    Value();
    virtual ~Value() = default;
    
    // Value type management
    enum class ValueType {
        INTEGER,
        DOUBLE,
        STRING,
        BOOLEAN,
        POINTER,
        UNDEFINED
    };
    
    ValueType getType() const { return type_; }
    void setType(ValueType type) { type_ = type; }
    
    // Value data access
    void setInt(int value);
    void setDouble(double value);
    void setString(const std::string& value);
    void setBool(bool value);
    void setPointer(void* value);
    
    int getInt() const;
    double getDouble() const;
    std::string getString() const;
    bool getBool() const;
    void* getPointer() const;
    
    // Value conversion
    std::string toString() const;
    bool toBool() const;
    int toInt() const;
    double toDouble() const;
    
    // Value validation
    bool isValid() const;
    void clear();
    
    // Value comparison
    bool equals(const Value& other) const;
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    
    // Value operations
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    
protected:
    ValueType type_;
    std::variant<int, double, std::string, bool, void*> data_;
    
    // Helper methods for type conversion
    void convertTo(ValueType target_type);
    bool canConvertTo(ValueType target_type) const;
};

#endif // VALUE_H

