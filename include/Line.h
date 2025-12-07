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

