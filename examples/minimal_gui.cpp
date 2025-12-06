#include <xtd/xtd>

using namespace xtd;
using namespace xtd::forms;

class main_form : public form {
public:
    main_form() {
        text("CAD System - Main Window");
        size({800, 600});
        start_position(form_start_position::center_screen);
        
        setup_menu();
        setup_controls();
    }
    
private:
    void setup_menu() {
        // New 2D Document
        new_2d_doc.text("&New 2D Document");
        new_2d_doc.shortcut(shortcut::ctrl_n);
        new_2d_doc.click += event_handler {*this, &main_form::on_new_2d_document};
        
        // Separator 1
        separator1.text("-");
        
        // Open
        open_doc.text("&Open...");
        open_doc.shortcut(shortcut::ctrl_o);
        open_doc.click += event_handler {*this, &main_form::on_open_document};
        
        // Save
        save_doc.text("&Save");
        save_doc.shortcut(shortcut::ctrl_s);
        save_doc.click += event_handler {*this, &main_form::on_save_document};
        
        // Separator 2
        separator2.text("-");
        
        // Exit
        exit_item.text("E&xit");
        exit_item.shortcut(shortcut::ctrl_q);
        exit_item.click += event_handler {*this, &main_form::on_exit};
        
        // File menu
        file_menu.text("&File");
        file_menu.menu_items().add(new_2d_doc);
        file_menu.menu_items().add(separator1);
        file_menu.menu_items().add(open_doc);
        file_menu.menu_items().add(save_doc);
        file_menu.menu_items().add(separator2);
        file_menu.menu_items().add(exit_item);
        
        // Help menu
        about_item.text("&About...");
        about_item.click += event_handler {*this, &main_form::on_about};
        
        help_menu.text("&Help");
        help_menu.menu_items().add(about_item);
        
        // Main menu
        main_menu1.menu_items().add(file_menu);
        main_menu1.menu_items().add(help_menu);
        menu(main_menu1);
    }
    
    void on_new_2d_document(object& sender, const event_args& e) {
        message_box::show("New 2D Document created!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                         "CAD System", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
        status_label.text("New 2D Document created");
    }
    
    void on_open_document(object& sender, const event_args& e) {
        open_file_dialog dialog;
        dialog.filter("CAD Documents (*.json)|*.json|All Files (*.*)|*.*");
        if (dialog.show_dialog(*this) == dialog_result::ok) {
            message_box::show("Opening: " + dialog.file_name(), "CAD System");
            status_label.text("Document opened: " + dialog.file_name());
        }
    }
    
    void on_save_document(object& sender, const event_args& e) {
        message_box::show("Document saved!", "CAD System");
        status_label.text("Document saved");
    }
    
    void on_exit(object& sender, const event_args& e) {
        application::exit();
    }
    
    void on_about(object& sender, const event_args& e) {
        message_box::show("CAD System v1.0\nDriver Solution CAD\n\nBuilt with xtd framework", 
                         "About CAD System", 
                         message_box_buttons::ok, 
                         message_box_icon::information);
    }
    
    void setup_controls() {
        // Status label
        status_label.parent(*this);
        status_label.text("Ready");
        status_label.location({10, height() - 30});
        status_label.size({width() - 20, 25});
        status_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::bottom);
        
        // Welcome label
        welcome_label.parent(*this);
        welcome_label.text("Welcome to CAD System!\n\nUse File -> New 2D Document to create a new document.");
        welcome_label.location({20, 20});
        welcome_label.size({width() - 40, 150});
        welcome_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::top);
        welcome_label.text_align(content_alignment::middle_center);
    }
    
    // Menu items as members to ensure they live long enough
    menu_item new_2d_doc;
    menu_item separator1;
    menu_item open_doc;
    menu_item save_doc;
    menu_item separator2;
    menu_item exit_item;
    menu_item file_menu;
    menu_item about_item;
    menu_item help_menu;
    main_menu main_menu1;
    
    label status_label;
    label welcome_label;
};

auto main() -> int {
    application::enable_visual_styles();
    application::run(main_form {});
    return 0;
}
