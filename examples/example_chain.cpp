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

