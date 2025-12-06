#include <xtd/xtd>
#include <iostream>

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
        // File Menu
        auto file_menu = menu_item::create("&File");
        
        auto new_2d_doc = menu_item::create("&New 2D Document", {keys::control | keys::n}, [this] {
            message_box::show("New 2D Document created!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                             "CAD System", 
                             message_box_buttons::ok, 
                             message_box_icon::information);
            status_label.text("New 2D Document created");
        });
        
        auto open_doc = menu_item::create("&Open...", {keys::control | keys::o}, [this] {
            open_file_dialog dialog;
            dialog.filter("CAD Documents (*.json)|*.json|All Files (*.*)|*.*");
            if (dialog.show_dialog(*this) == dialog_result::ok) {
                message_box::show("Opening: " + dialog.file_name(), "CAD System");
                status_label.text("Document opened: " + dialog.file_name());
            }
        });
        
        auto save_doc = menu_item::create("&Save", {keys::control | keys::s}, [this] {
            message_box::show("Document saved!", "CAD System");
            status_label.text("Document saved");
        });
        
        auto save_as_doc = menu_item::create("Save &As...", {keys::control | keys::shift | keys::s}, [this] {
            save_file_dialog dialog;
            dialog.filter("CAD Documents (*.json)|*.json|All Files (*.*)|*.*");
            if (dialog.show_dialog(*this) == dialog_result::ok) {
                message_box::show("Saving to: " + dialog.file_name(), "CAD System");
                status_label.text("Document saved as: " + dialog.file_name());
            }
        });
        
        auto close_doc = menu_item::create("&Close", {keys::control | keys::w}, [this] {
            status_label.text("Document closed");
        });
        
        auto separator1 = menu_item::create_separator();
        
        auto exit_item = menu_item::create("E&xit", {keys::control | keys::q}, [this] {
            application::exit();
        });
        
        file_menu->menu_items().push_back(new_2d_doc);
        file_menu->menu_items().push_back(open_doc);
        file_menu->menu_items().push_back(save_doc);
        file_menu->menu_items().push_back(save_as_doc);
        file_menu->menu_items().push_back(close_doc);
        file_menu->menu_items().push_back(separator1);
        file_menu->menu_items().push_back(exit_item);
        
        // Edit Menu
        auto edit_menu = menu_item::create("&Edit");
        edit_menu->menu_items().push_back(menu_item::create("&Undo", {keys::control | keys::z}));
        edit_menu->menu_items().push_back(menu_item::create("&Redo", {keys::control | keys::y}));
        edit_menu->menu_items().push_back(menu_item::create_separator());
        edit_menu->menu_items().push_back(menu_item::create("Cu&t", {keys::control | keys::x}));
        edit_menu->menu_items().push_back(menu_item::create("&Copy", {keys::control | keys::c}));
        edit_menu->menu_items().push_back(menu_item::create("&Paste", {keys::control | keys::v}));
        
        // View Menu
        auto view_menu = menu_item::create("&View");
        view_menu->menu_items().push_back(menu_item::create("Zoom &In", {keys::control | keys::add}));
        view_menu->menu_items().push_back(menu_item::create("Zoom &Out", {keys::control | keys::subtract}));
        view_menu->menu_items().push_back(menu_item::create("&Fit to Window", {keys::control | keys::d0}));
        view_menu->menu_items().push_back(menu_item::create("&Actual Size", {keys::control | keys::d1}));
        
        // Tools Menu
        auto tools_menu = menu_item::create("&Tools");
        tools_menu->menu_items().push_back(menu_item::create("&Options..."));
        tools_menu->menu_items().push_back(menu_item::create("&Terminal...", [this] {
            message_box::show("Terminal window would open here", "CAD System");
        }));
        
        // Help Menu
        auto help_menu = menu_item::create("&Help");
        help_menu->menu_items().push_back(menu_item::create("&About...", [this] {
            message_box::show("CAD System v1.0\nDriver Solution CAD\n\nBuilt with xtd framework", 
                             "About CAD System", 
                             message_box_buttons::ok, 
                             message_box_icon::information);
        }));
        help_menu->menu_items().push_back(menu_item::create("&Documentation"));
        
        // Create menu bar
        menu_bar main_menu;
        main_menu.menu_items().push_back(file_menu);
        main_menu.menu_items().push_back(edit_menu);
        main_menu.menu_items().push_back(view_menu);
        main_menu.menu_items().push_back(tools_menu);
        main_menu.menu_items().push_back(help_menu);
        
        menu(main_menu);
    }
    
    void setup_controls() {
        // Status bar
        status_label.parent(*this);
        status_label.text("Ready");
        status_label.location({0, height() - 25});
        status_label.size({width(), 25});
        status_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::bottom);
        
        // Welcome label
        welcome_label.parent(*this);
        welcome_label.text("Welcome to CAD System!\n\nUse File -> New 2D Document to create a new document.");
        welcome_label.location({20, 20});
        welcome_label.size({width() - 40, 100});
        welcome_label.anchor(anchor_styles::left | anchor_styles::right | anchor_styles::top);
        welcome_label.text_align(content_alignment::middle_center);
    }
    
    label status_label;
    label welcome_label;
};

auto main() -> int {
    application::enable_visual_styles();
    application::set_compatible_text_rendering_default(false);
    application::run(main_form {});
    return 0;
}


