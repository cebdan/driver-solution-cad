#include "../include/MainWindow.h"
#include <xtd/xtd>
#include <iostream>

// Full implementation with xtd GUI menu
class MainWindowWithMenu {
private:
    xtd::forms::form main_form_;
    xtd::forms::menu_bar menu_bar_;
    xtd::forms::menu_item file_menu_;
    xtd::forms::menu_item edit_menu_;
    xtd::forms::menu_item view_menu_;
    xtd::forms::menu_item tools_menu_;
    xtd::forms::menu_item help_menu_;
    
    std::function<void()> on_new_2d_document_;
    std::function<void()> on_open_document_;
    std::function<void()> on_save_document_;
    std::function<void()> on_exit_;
    
public:
    MainWindowWithMenu() {
        setupForm();
        setupMenuBar();
    }
    
    void setupForm() {
        main_form_.text("CAD System");
        main_form_.size({800, 600});
        main_form_.start_position(xtd::forms::form_start_position::center_screen);
    }
    
    void setupMenuBar() {
        // File Menu
        file_menu_.text("&File");
        
        auto new_2d_doc_item = xtd::forms::menu_item::create("&New 2D Document", {xtd::forms::keys::control | xtd::forms::keys::n}, [this] {
            onNew2DDocument();
        });
        
        auto open_item = xtd::forms::menu_item::create("&Open...", {xtd::forms::keys::control | xtd::forms::keys::o}, [this] {
            onOpenDocument();
        });
        
        auto save_item = xtd::forms::menu_item::create("&Save", {xtd::forms::keys::control | xtd::forms::keys::s}, [this] {
            onSaveDocument();
        });
        
        auto save_as_item = xtd::forms::menu_item::create("Save &As...", {xtd::forms::keys::control | xtd::forms::keys::shift | xtd::forms::keys::s});
        
        auto close_item = xtd::forms::menu_item::create("&Close", {xtd::forms::keys::control | xtd::forms::keys::w});
        
        auto separator1 = xtd::forms::menu_item::create_separator();
        
        auto exit_item = xtd::forms::menu_item::create("E&xit", {xtd::forms::keys::control | xtd::forms::keys::q}, [this] {
            onExit();
        });
        
        file_menu_.menu_items().push_back(new_2d_doc_item);
        file_menu_.menu_items().push_back(open_item);
        file_menu_.menu_items().push_back(save_item);
        file_menu_.menu_items().push_back(save_as_item);
        file_menu_.menu_items().push_back(close_item);
        file_menu_.menu_items().push_back(separator1);
        file_menu_.menu_items().push_back(exit_item);
        
        // Edit Menu
        edit_menu_.text("&Edit");
        
        auto undo_item = xtd::forms::menu_item::create("&Undo", {xtd::forms::keys::control | xtd::forms::keys::z});
        auto redo_item = xtd::forms::menu_item::create("&Redo", {xtd::forms::keys::control | xtd::forms::keys::y});
        auto separator2 = xtd::forms::menu_item::create_separator();
        auto cut_item = xtd::forms::menu_item::create("Cu&t", {xtd::forms::keys::control | xtd::forms::keys::x});
        auto copy_item = xtd::forms::menu_item::create("&Copy", {xtd::forms::keys::control | xtd::forms::keys::c});
        auto paste_item = xtd::forms::menu_item::create("&Paste", {xtd::forms::keys::control | xtd::forms::keys::v});
        
        edit_menu_.menu_items().push_back(undo_item);
        edit_menu_.menu_items().push_back(redo_item);
        edit_menu_.menu_items().push_back(separator2);
        edit_menu_.menu_items().push_back(cut_item);
        edit_menu_.menu_items().push_back(copy_item);
        edit_menu_.menu_items().push_back(paste_item);
        
        // View Menu
        view_menu_.text("&View");
        
        auto zoom_in_item = xtd::forms::menu_item::create("Zoom &In", {xtd::forms::keys::control | xtd::forms::keys::add});
        auto zoom_out_item = xtd::forms::menu_item::create("Zoom &Out", {xtd::forms::keys::control | xtd::forms::keys::subtract});
        auto fit_window_item = xtd::forms::menu_item::create("&Fit to Window", {xtd::forms::keys::control | xtd::forms::keys::d0});
        auto actual_size_item = xtd::forms::menu_item::create("&Actual Size", {xtd::forms::keys::control | xtd::forms::keys::d1});
        
        view_menu_.menu_items().push_back(zoom_in_item);
        view_menu_.menu_items().push_back(zoom_out_item);
        view_menu_.menu_items().push_back(fit_window_item);
        view_menu_.menu_items().push_back(actual_size_item);
        
        // Tools Menu
        tools_menu_.text("&Tools");
        
        auto options_item = xtd::forms::menu_item::create("&Options...");
        auto terminal_item = xtd::forms::menu_item::create("&Terminal...");
        
        tools_menu_.menu_items().push_back(options_item);
        tools_menu_.menu_items().push_back(terminal_item);
        
        // Help Menu
        help_menu_.text("&Help");
        
        auto about_item = xtd::forms::menu_item::create("&About...");
        auto docs_item = xtd::forms::menu_item::create("&Documentation");
        
        help_menu_.menu_items().push_back(about_item);
        help_menu_.menu_items().push_back(docs_item);
        
        // Add menus to menu bar
        menu_bar_.menu_items().push_back(file_menu_);
        menu_bar_.menu_items().push_back(edit_menu_);
        menu_bar_.menu_items().push_back(view_menu_);
        menu_bar_.menu_items().push_back(tools_menu_);
        menu_bar_.menu_items().push_back(help_menu_);
        
        // Set menu bar to form
        main_form_.menu(menu_bar_);
    }
    
    void onNew2DDocument() {
        std::cout << "[Menu] New 2D Document selected" << std::endl;
        if (on_new_2d_document_) {
            on_new_2d_document_();
        }
    }
    
    void onOpenDocument() {
        std::cout << "[Menu] Open Document selected" << std::endl;
        if (on_open_document_) {
            on_open_document_();
        }
    }
    
    void onSaveDocument() {
        std::cout << "[Menu] Save Document selected" << std::endl;
        if (on_save_document_) {
            on_save_document_();
        }
    }
    
    void onExit() {
        if (on_exit_) {
            on_exit_();
        } else {
            xtd::forms::application::exit();
        }
    }
    
    void setOnNew2DDocument(std::function<void()> callback) {
        on_new_2d_document_ = callback;
    }
    
    void setOnOpenDocument(std::function<void()> callback) {
        on_open_document_ = callback;
    }
    
    void setOnSaveDocument(std::function<void()> callback) {
        on_save_document_ = callback;
    }
    
    void setOnExit(std::function<void()> callback) {
        on_exit_ = callback;
    }
    
    void show() {
        main_form_.show();
    }
    
    xtd::forms::form& getForm() {
        return main_form_;
    }
};

// Example usage
auto main() -> int {
    xtd::forms::application::enable_visual_styles();
    xtd::forms::application::set_compatible_text_rendering_default(false);
    
    MainWindowWithMenu mainWindow;
    
    mainWindow.setOnNew2DDocument([]() {
        xtd::forms::message_box::show("New 2D Document created!", "CAD System");
    });
    
    mainWindow.setOnOpenDocument([]() {
        xtd::forms::message_box::show("Open Document dialog would appear here", "CAD System");
    });
    
    mainWindow.setOnSaveDocument([]() {
        xtd::forms::message_box::show("Document saved!", "CAD System");
    });
    
    mainWindow.show();
    xtd::forms::application::run();
    
    return 0;
}


