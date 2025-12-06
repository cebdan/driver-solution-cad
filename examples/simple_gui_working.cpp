#include <xtd/xtd>
#include "../include/close_app.h"
#ifdef __APPLE__
#include "../include/macos_window_utils.h"
#include <thread>
#include <chrono>
#endif
#include <cstdlib>
#include <string>
#include <filesystem>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

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
        
        // On macOS, setup style for Solution manager window only
        #if defined(__APPLE__)
        application::do_events();
        setup_solution_manager_window_style();
        // Also try direct method as backup
        hide_close_button_by_title("Solution manager");
        #endif
    }
    
    void update_status(const std::string& text) {
        status_label.text(text);
    }
    
private:
    
    
    void setup_controls() {
        status_label.parent(*this);
        status_label.text("Ready");
        status_label.location({10, height() - 30});
        status_label.size({width() - 20, 25});
        status_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::bottom);
        
        welcome_label.parent(*this);
        welcome_label.text("Welcome to CAD System!\n\nUse File -> New 2D Document to create a new document.");
        welcome_label.location({20, 20});
        welcome_label.size({width() - 40, 150});
        welcome_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::top);
        welcome_label.text_align(content_alignment::middle_center);
    }
    
    label status_label;
    label welcome_label;
    
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
    
    // On macOS, setup style for Solution manager window only (after it's created)
    #if defined(__APPLE__)
    application::do_events();
    setup_solution_manager_window_style();
    #endif
    
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
    
    static menu_item app_solution_2d = menu_item {"&2D Solution", [](object& sender, const event_args& e) {
        if (g_main_form) {
            g_main_form->show_manager();
            message_box::show("2D Solution activated!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                             "Solution Manager", 
                             message_box_buttons::ok, 
                             message_box_icon::information);
            g_main_form->update_status("2D Solution activated");
        }
    }};
    g_app_solution_2d = &app_solution_2d;
    
    static menu_item app_manager_menu = menu_item {"&Manager", {app_open_manager, app_solution_2d}};
    
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
        
        // 3. Try relative to executable (for .app bundle on macOS)
        #if defined(__APPLE__)
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
        #endif
        
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
            #if defined(__APPLE__)
            command = "open \"" + path_str + "\"";
            #elif defined(_WIN32)
            command = "start \"\" \"" + path_str + "\"";
            #else
            command = "xdg-open \"" + path_str + "\"";
            #endif
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
