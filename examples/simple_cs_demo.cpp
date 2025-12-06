#include <iostream>
#include <string>
#include <cmath>

// Simplified CS class for demonstration
class SimpleCS {
public:
    SimpleCS() : x_(0.0), y_(0.0), z_(0.0), a_(0.0), b_(0.0), c_(0.0), name_("CS") {}
    SimpleCS(double x, double y, double z, double a = 0.0, double b = 0.0, double c = 0.0)
        : x_(x), y_(y), z_(z), a_(a), b_(b), c_(c), name_("CS") {}
    
    void setName(const std::string& name) { name_ = name; }
    std::string getName() const { return name_; }
    
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    double getA() const { return a_; }
    double getB() const { return b_; }
    double getC() const { return c_; }
    
    double distanceToOrigin() const {
        return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
    }
    
    double distance(const SimpleCS& other) const {
        double dx = x_ - other.x_;
        double dy = y_ - other.y_;
        double dz = z_ - other.z_;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    std::string toString() const {
        return name_ + "(" + std::to_string(x_) + ", " + std::to_string(y_) + ", " + 
               std::to_string(z_) + ", " + std::to_string(a_) + ", " + 
               std::to_string(b_) + ", " + std::to_string(c_) + ")";
    }
    
    static SimpleCS GlobalCS() {
        SimpleCS global(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        global.setName("Global CS");
        return global;
    }
    
private:
    double x_, y_, z_;
    double a_, b_, c_;
    std::string name_;
};

int main() {
    // Create Global CS (0,0,0,0,0,0)
    SimpleCS globalCS = SimpleCS::GlobalCS();
    
    std::cout << "=== Global Coordinate System ===" << std::endl;
    std::cout << "Name: " << globalCS.getName() << std::endl;
    std::cout << "Position: (" << globalCS.getX() << ", " 
              << globalCS.getY() << ", " << globalCS.getZ() << ")" << std::endl;
    std::cout << "Rotation: (" << globalCS.getA() << ", " 
              << globalCS.getB() << ", " << globalCS.getC() << ")" << std::endl;
    std::cout << "String representation: " << globalCS.toString() << std::endl;
    std::cout << "Distance to origin: " << globalCS.distanceToOrigin() << std::endl;
    
    // Create another CS and calculate distance
    SimpleCS otherCS(10.0, 20.0, 30.0, 0.0, 0.0, 0.0);
    otherCS.setName("Other CS");
    
    std::cout << "\n=== Other Coordinate System ===" << std::endl;
    std::cout << "Name: " << otherCS.getName() << std::endl;
    std::cout << "Position: (" << otherCS.getX() << ", " 
              << otherCS.getY() << ", " << otherCS.getZ() << ")" << std::endl;
    std::cout << "Distance from Global CS: " << globalCS.distance(otherCS) << std::endl;
    
    // Create CS with rotation
    SimpleCS rotatedCS(5.0, 5.0, 5.0, 0.785, 0.0, 0.0); // 45 degrees rotation
    rotatedCS.setName("Rotated CS");
    
    std::cout << "\n=== Rotated Coordinate System ===" << std::endl;
    std::cout << "Name: " << rotatedCS.getName() << std::endl;
    std::cout << "Position: (" << rotatedCS.getX() << ", " 
              << rotatedCS.getY() << ", " << rotatedCS.getZ() << ")" << std::endl;
    std::cout << "Rotation: (" << rotatedCS.getA() << ", " 
              << rotatedCS.getB() << ", " << rotatedCS.getC() << ")" << std::endl;
    std::cout << "Distance from Global CS: " << globalCS.distance(rotatedCS) << std::endl;
    
    return 0;
}

