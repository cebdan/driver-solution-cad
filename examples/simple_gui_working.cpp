#include <xtd/xtd>
#include "../include/close_app.h"
#include "../include/macos_window_utils.h"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <mach-o/dyld.h>

using namespace xtd;
using namespace xtd::forms;

// Global reference to the form so menu can access it
static class main_form* g_main_form = nullptr;

class main_form : public form {
public:
    main_form() {
        g_main_form = this;
        text("Solution manager");
        size({800, 600});
        start_position(form_start_position::center_screen);
        
        // Disable close button (red) and maximize button (green) - only minimize button (yellow) will be available
        minimize_box(true);
        maximize_box(false);
        
        // On macOS, we'll set window style when window is created
        // This will be done in show_manager() before showing
        
        // Handle window closing - hide window instead of closing it
        // But since we disabled close button, this should not be triggered by red button
        form_closing += [](object& sender, form_closing_event_args& e) {
            e.cancel(true); // Cancel the close event
            static_cast<form&>(sender).hide(); // Hide the window instead
        };
        
        
        // Handle Cmd+Q (Quit) key combination for macOS
        // On macOS, Control key is automatically mapped to Command key
        key_down += [](object& sender, key_event_args& e) {
            // Check for Control+Q (which becomes Cmd+Q on macOS)
            if ((e.modifiers() & keys::control) != keys::none && e.key_code() == keys::q) {
                close_app::close();
            }
        };
        
        setup_controls();
    }
    
    void show_manager() {
        show();
        bring_to_front();
        
        // Setup style for Solution manager window only - native macOS
        application::do_events();
        setup_solution_manager_window_style();
        // Also try direct method as backup
        hide_close_button_by_title("Solution manager");
    }
    
    void update_status(const std::string& text) {
        status_label.text(text);
    }
    
    // Button click handlers (public for menu access)
    void on_2d_solutions_click() {
        status_label.text("2D Solutions activated");
        // Minimize the manager window first (keeps menu visible) - native macOS
        minimize_window_by_title("Solution manager");
        // Then show message box
        message_box::show("2D Solutions activated!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                         "Solution Manager", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
    }
    
    void on_3d_solutions_click() {
        message_box::show("3D Solutions activated!", 
                         "Solution Manager", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
        status_label.text("3D Solutions activated");
    }
    
    void on_draft_solutions_click() {
        message_box::show("Draft Solutions activated!", 
                         "Solution Manager", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
        status_label.text("Draft Solutions activated");
    }
    
    void on_bom_solution_click() {
        message_box::show("BOM Solution activated!", 
                         "Solution Manager", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
        status_label.text("BOM Solution activated");
    }
    
private:
    
    
    void setup_controls() {
        // Status label at the bottom
        status_label.parent(*this);
        status_label.text("Ready");
        status_label.location({10, height() - 30});
        status_label.size({width() - 20, 25});
        status_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::bottom);
        
        // Main content area (left side) - image display
        image_panel.parent(*this);
        image_panel.location({20, 20});
        image_panel.size({width() - 250, height() - 80});
        image_panel.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::top | anchor_styles::bottom);
        image_panel.border_style(border_style::none);
        image_panel.back_color(drawing::color::white);
        
        // Try to load image if exists
        load_image();
        
        // Buttons on the right side
        int button_width = 200;
        int button_height = 40;
        int button_x = width() - button_width - 20;
        int button_y = 50;
        int button_spacing = 50;
        
        // 2D Solutions button
        button_2d_solutions.parent(*this);
        button_2d_solutions.text("2d solutions");
        button_2d_solutions.location({button_x, button_y});
        button_2d_solutions.size({button_width, button_height});
        button_2d_solutions.anchor(anchor_styles::right | anchor_styles::top);
        button_2d_solutions.click += [this](object& sender, const event_args& e) {
            on_2d_solutions_click();
        };
        
        // 3D Solutions button
        button_3d_solutions.parent(*this);
        button_3d_solutions.text("3d solutions");
        button_3d_solutions.location({button_x, button_y + button_spacing});
        button_3d_solutions.size({button_width, button_height});
        button_3d_solutions.anchor(anchor_styles::right | anchor_styles::top);
        button_3d_solutions.click += [this](object& sender, const event_args& e) {
            on_3d_solutions_click();
        };
        
        // Draft Solutions button
        button_draft_solutions.parent(*this);
        button_draft_solutions.text("Draft solutions");
        button_draft_solutions.location({button_x, button_y + button_spacing * 2});
        button_draft_solutions.size({button_width, button_height});
        button_draft_solutions.anchor(anchor_styles::right | anchor_styles::top);
        button_draft_solutions.click += [this](object& sender, const event_args& e) {
            on_draft_solutions_click();
        };
        
        // BOM Solution button
        button_bom_solution.parent(*this);
        button_bom_solution.text("Bom solution");
        button_bom_solution.location({button_x, button_y + button_spacing * 3});
        button_bom_solution.size({button_width, button_height});
        button_bom_solution.anchor(anchor_styles::right | anchor_styles::top);
        button_bom_solution.click += [this](object& sender, const event_args& e) {
            on_bom_solution_click();
        };
        
        // Load Image button (below BOM)
        button_load_image.parent(*this);
        button_load_image.text("Load Image");
        button_load_image.location({button_x, button_y + button_spacing * 4 + 20});
        button_load_image.size({button_width, button_height});
        button_load_image.anchor(anchor_styles::right | anchor_styles::top);
        button_load_image.click += [this](object& sender, const event_args& e) {
            open_image_dialog();
        };
    }
    
    void load_image() {
        // Try to find image file in project directory
        std::vector<std::filesystem::path> search_paths;
        
        // Current directory
        auto cwd = std::filesystem::current_path();
        search_paths.push_back(cwd / "draft_image.png");
        search_paths.push_back(cwd / "draft_image.jpg");
        search_paths.push_back(cwd / "image.png");
        search_paths.push_back(cwd / "image.jpg");
        
        // Parent directory (if in build)
        if (cwd.filename() == "build") {
            search_paths.push_back(cwd.parent_path() / "draft_image.png");
            search_paths.push_back(cwd.parent_path() / "draft_image.jpg");
            search_paths.push_back(cwd.parent_path() / "image.png");
            search_paths.push_back(cwd.parent_path() / "image.jpg");
        }
        
        // Try to load image
        for (const auto& path : search_paths) {
            if (std::filesystem::exists(path)) {
                try {
                    drawing::bitmap bmp(path.string());
                    image_panel.image(bmp);
                    image_panel.size_mode(picture_box_size_mode::zoom);
                    return;
                } catch (...) {
                    // If image loading fails, continue to next path
                }
            }
        }
        
        // If no image found, leave empty (will show white background with border)
    }
    
    void load_image_from_file(const std::string& file_path) {
        if (std::filesystem::exists(file_path)) {
            try {
                drawing::bitmap bmp(file_path);
                image_panel.image(bmp);
                image_panel.size_mode(picture_box_size_mode::zoom);
                status_label.text("Image loaded: " + std::filesystem::path(file_path).filename().string());
            } catch (...) {
                message_box::show("Failed to load image: " + file_path, 
                                 "Error", 
                                 message_box_buttons::ok, 
                                 message_box_icon::error);
            }
        }
    }
    
    void open_image_dialog() {
        open_file_dialog dialog;
        dialog.filter("Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.gif)|*.png;*.jpg;*.jpeg;*.bmp;*.gif|All Files (*.*)|*.*");
        dialog.title("Select Image");
        if (dialog.show_dialog(*this) == dialog_result::ok) {
            load_image_from_file(dialog.file_name());
        }
    }
    
    label status_label;
    picture_box image_panel;
    button button_2d_solutions;
    button button_3d_solutions;
    button button_draft_solutions;
    button button_bom_solution;
    button button_load_image;
    
    friend auto main() -> int;
};

// Static menu items to keep them alive even when window is hidden
static menu_item* g_app_new_2d_doc = nullptr;
static menu_item* g_app_open_doc = nullptr;
static menu_item* g_app_save_doc = nullptr;
static menu_item* g_app_quit = nullptr;
static menu_item* g_app_open_manager = nullptr;
static menu_item* g_app_solution_2d = nullptr;
static menu_item* g_app_about = nullptr;
static main_menu* g_app_main_menu = nullptr;

auto main() -> int {
    application::enable_visual_styles();
    
    // Create the main form as static to keep it alive
    static main_form form;
    form.show();
    
    // Setup style for Solution manager window only (after it's created) - native macOS
    application::do_events();
    setup_solution_manager_window_style();
    
    // Create global menu items as static to keep them alive even when window is hidden
    // File menu
    static menu_item app_new_2d_doc = menu_item {"&New 2D Document", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            message_box::show("New 2D Document created!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                             "CAD System", 
                             message_box_buttons::ok, 
                             message_box_icon::information);
            g_main_form->update_status("New 2D Document created");
        }
    }, shortcut::ctrl_n};
    g_app_new_2d_doc = &app_new_2d_doc;
    
    static menu_item app_separator1 = menu_item {"-"};
    
    static menu_item app_open_doc = menu_item {"&Open...", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            open_file_dialog dialog;
            dialog.filter("CAD Documents (*.json)|*.json|All Files (*.*)|*.*");
            if (dialog.show_dialog(*g_main_form) == dialog_result::ok) {
                message_box::show("Opening: " + dialog.file_name(), "CAD System");
                g_main_form->update_status("Document opened: " + dialog.file_name());
            }
        }
    }, shortcut::ctrl_o};
    g_app_open_doc = &app_open_doc;
    
    static menu_item app_save_doc = menu_item {"&Save", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            message_box::show("Document saved!", "CAD System");
            g_main_form->update_status("Document saved");
        }
    }, shortcut::ctrl_s};
    g_app_save_doc = &app_save_doc;
    
    static menu_item app_separator2 = menu_item {"-"};
    
    // Quit menu item - shortcut::ctrl_q should work on macOS as Cmd+Q
    static menu_item app_quit = menu_item {"&Quit", [](object& sender, const event_args& e) {
        close_app::close();
    }, shortcut::ctrl_q};
    g_app_quit = &app_quit;
    
    static menu_item app_file_menu = menu_item {"&File", {app_new_2d_doc, app_separator1, app_open_doc, app_save_doc, app_separator2, app_quit}};
    
    // Manager menu - this stays available even when window is closed
    static menu_item app_open_manager = menu_item {"&Open Manager", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            g_main_form->update_status("Manager opened");
        }
    }};
    g_app_open_manager = &app_open_manager;
    
    static menu_item app_solution_2d = menu_item {"&2D Solutions", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            g_main_form->on_2d_solutions_click();
        }
    }};
    g_app_solution_2d = &app_solution_2d;
    
    static menu_item app_solution_3d = menu_item {"&3D Solutions", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            g_main_form->on_3d_solutions_click();
        }
    }};
    
    static menu_item app_draft_solutions = menu_item {"&Draft Solutions", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            g_main_form->on_draft_solutions_click();
        }
    }};
    
    static menu_item app_bom_solution = menu_item {"&BOM Solution", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            g_main_form->on_bom_solution_click();
        }
    }};
    
    static menu_item app_manager_menu = menu_item {"&Manager", {app_open_manager, app_solution_2d, app_solution_3d, app_draft_solutions, app_bom_solution}};
    
    // Function to open document file using system default application
    auto open_document = [](const std::string& filename) {
        std::filesystem::path doc_path;
        
        // Try multiple locations
        std::vector<std::filesystem::path> search_paths;
        
        // 1. Current working directory
        search_paths.push_back(std::filesystem::current_path() / filename);
        
        // 2. If in build directory, try parent
        auto cwd = std::filesystem::current_path();
        if (cwd.filename() == "build") {
            search_paths.push_back(cwd.parent_path() / filename);
        }
        
        // 3. Try relative to executable (for .app bundle on macOS) - native macOS
        char exe_path[1024];
        uint32_t size = sizeof(exe_path);
        if (_NSGetExecutablePath(exe_path, &size) == 0) {
            std::filesystem::path exe = exe_path;
            // Go up from Contents/MacOS/executable to .app, then to project root
            auto app_path = exe.parent_path().parent_path().parent_path();
            search_paths.push_back(app_path / filename);
            // Also try parent of .app (project root)
            if (app_path.extension() == ".app") {
                search_paths.push_back(app_path.parent_path() / filename);
            }
        }
        
        // 4. Try absolute path if filename is absolute
        if (std::filesystem::path(filename).is_absolute()) {
            search_paths.push_back(std::filesystem::path(filename));
        }
        
        // Find first existing file
        bool found = false;
        for (const auto& path : search_paths) {
            if (std::filesystem::exists(path)) {
                doc_path = path;
                found = true;
                break;
            }
        }
        
        if (found) {
            std::string command;
            std::string path_str = doc_path.string();
            // Native macOS command
            command = "open \"" + path_str + "\"";
            std::system(command.c_str());
        } else {
            message_box::show("Document not found: " + filename, 
                             "Error", 
                             message_box_buttons::ok, 
                             message_box_icon::error);
        }
    };
    
    // Help menu items
    static menu_item app_implementation_notes = menu_item {"&Implementation Notes", [open_document](object& sender, const event_args& e) {
        open_document("IMPLEMENTATION_NOTES.md");
    }};
    
    static menu_item app_project_goals = menu_item {"&Project Goals", [open_document](object& sender, const event_args& e) {
        open_document("PROJECT_GOALS.md");
    }};
    
    static menu_item app_help_separator = menu_item {"-"};
    
    static menu_item app_about = menu_item {"&About...", [](object& sender, const event_args& e) {
        message_box::show("CAD System v1.0\nDriver Solution CAD\n\nBuilt with xtd framework", 
                         "About CAD System", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
    }};
    g_app_about = &app_about;
    
    static menu_item app_help_menu = menu_item {"&Help", {app_implementation_notes, app_project_goals, app_help_separator, app_about}};
    
    // Set menu on the form - menu items are static so they persist
    static main_menu app_main_menu = main_menu {app_file_menu, app_manager_menu, app_help_menu};
    g_app_main_menu = &app_main_menu;
    form.menu(app_main_menu);
    
    // Run application loop - this keeps the app running even if window is closed
    // Menu should remain available because menu items are static
    application::run();
    
    return 0;
}
