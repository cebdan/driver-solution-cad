#include "../include/CS.h"
#include <iostream>

int main() {
    // Create Global CS (0,0,0,0,0,0)
    CS globalCS = CS::GlobalCS();
    
    std::cout << "=== Global Coordinate System ===" << std::endl;
    std::cout << "Name: " << globalCS.getName() << std::endl;
    std::cout << "Position: (" << globalCS.getX() << ", " 
              << globalCS.getY() << ", " << globalCS.getZ() << ")" << std::endl;
    std::cout << "Rotation: (" << globalCS.getA() << ", " 
              << globalCS.getB() << ", " << globalCS.getC() << ")" << std::endl;
    std::cout << "String representation: " << globalCS.toString() << std::endl;
    std::cout << "Is valid: " << (globalCS.isValid() ? "yes" : "no") << std::endl;
    std::cout << "Distance to origin: " << globalCS.distanceToOrigin() << std::endl;
    
    // Create another CS and calculate distance
    CS otherCS(10.0, 20.0, 30.0, 0.0, 0.0, 0.0);
    otherCS.setName("Other CS");
    
    std::cout << "\n=== Other Coordinate System ===" << std::endl;
    std::cout << "Name: " << otherCS.getName() << std::endl;
    std::cout << "Position: (" << otherCS.getX() << ", " 
              << otherCS.getY() << ", " << otherCS.getZ() << ")" << std::endl;
    std::cout << "Distance from Global CS: " << globalCS.distance(otherCS) << std::endl;
    
    // Test operations
    CS transformed = globalCS.transform(otherCS);
    std::cout << "\n=== Transformed CS ===" << std::endl;
    std::cout << "Transformed position: (" << transformed.getX() << ", " 
              << transformed.getY() << ", " << transformed.getZ() << ")" << std::endl;
    
    return 0;
}

