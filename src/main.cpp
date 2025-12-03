#include "UI/Application.h"
#include <iostream>
#include <exception>

int main() {
    try {
        CADCore::Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

