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
