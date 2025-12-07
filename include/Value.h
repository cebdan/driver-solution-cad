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
