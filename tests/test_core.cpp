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

