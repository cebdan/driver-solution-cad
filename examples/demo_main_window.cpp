#include <iostream>
#include <string>
#include <memory>

// Simplified MainWindow for demonstration
class SimpleMainWindow {
public:
    SimpleMainWindow() : title_("CAD System - Main Window") {}
    
    void show() {
        std::cout << "\n=== " << title_ << " ===" << std::endl;
        std::cout << "\nMenu:" << std::endl;
        std::cout << "  File" << std::endl;
        std::cout << "    -> New 2D Document" << std::endl;
        std::cout << "    -> Open Document" << std::endl;
        std::cout << "    -> Save Document" << std::endl;
        std::cout << "    -> Close Document" << std::endl;
        std::cout << "    -> Exit" << std::endl;
        std::cout << "\n  Edit" << std::endl;
        std::cout << "  View" << std::endl;
        std::cout << "  Tools" << std::endl;
        std::cout << "  Help" << std::endl;
    }
    
    void createNew2DDocument() {
        std::cout << "\n[Action] Creating new 2D document..." << std::endl;
        std::cout << "New 2D document created successfully!" << std::endl;
        std::cout << "Document name: New 2D Document" << std::endl;
        std::cout << "Default coordinate system: Global CS (0,0,0,0,0,0)" << std::endl;
    }
    
    void showMenu() {
        std::cout << "\nSelect menu item:" << std::endl;
        std::cout << "1. New 2D Document" << std::endl;
        std::cout << "2. Open Document" << std::endl;
        std::cout << "3. Save Document" << std::endl;
        std::cout << "4. Close Document" << std::endl;
        std::cout << "5. Exit" << std::endl;
        std::cout << "Enter choice (1-5): ";
    }
    
private:
    std::string title_;
};

int main() {
    SimpleMainWindow mainWindow;
    
    std::cout << "=== CAD System Main Window Demo ===" << std::endl;
    mainWindow.show();
    
    std::cout << "\n--- Testing Menu Actions ---" << std::endl;
    
    // Test creating new 2D document
    mainWindow.createNew2DDocument();
    
    std::cout << "\n--- Interactive Menu ---" << std::endl;
    mainWindow.showMenu();
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1:
            mainWindow.createNew2DDocument();
            break;
        case 2:
            std::cout << "[Action] Opening document..." << std::endl;
            break;
        case 3:
            std::cout << "[Action] Saving document..." << std::endl;
            break;
        case 4:
            std::cout << "[Action] Closing document..." << std::endl;
            break;
        case 5:
            std::cout << "[Action] Exiting application..." << std::endl;
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
    }
    
    return 0;
}

