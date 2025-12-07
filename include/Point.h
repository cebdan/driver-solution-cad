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

