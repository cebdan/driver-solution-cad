#include "../include/MainWindow.h"
#include <xtd/xtd>
#include <iostream>

// Demo application with main menu
auto main() -> int {
    try {
        // Enable visual styles
        xtd::forms::application::enable_visual_styles();
        xtd::forms::application::set_compatible_text_rendering_default(false);
        
        // Create main window
        MainWindow mainWindow;
        mainWindow.setTitle("CAD System - Main Window");
        
        // Setup event handlers
        mainWindow.setOnNew2DDocument([]() {
            std::cout << "Creating new 2D document..." << std::endl;
            xtd::forms::message_box::show("New 2D Document created!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                                         "CAD System", 
                                         xtd::forms::message_box_buttons::ok, 
                                         xtd::forms::message_box_icon::information);
        });
        
        mainWindow.setOnOpenDocument([]() {
            std::cout << "Opening document..." << std::endl;
            xtd::forms::message_box::show("Open Document dialog would appear here", 
                                         "CAD System");
        });
        
        mainWindow.setOnSaveDocument([]() {
            std::cout << "Saving document..." << std::endl;
            xtd::forms::message_box::show("Document saved successfully!", 
                                         "CAD System");
        });
        
        mainWindow.setOnCloseDocument([]() {
            std::cout << "Closing document..." << std::endl;
        });
        
        mainWindow.setOnExit([]() {
            std::cout << "Exiting application..." << std::endl;
            xtd::forms::application::exit();
        });
        
        // Show main window
        mainWindow.show();
        
        // Run application
        xtd::forms::application::run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        xtd::forms::message_box::show("Error: " + std::string(e.what()), "Error");
        return 1;
    }
    
    return 0;
}


