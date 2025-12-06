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
        menu_item file_menu;
        file_menu.text("&File");
        
        // New 2D Document menu item
        menu_item new_2d_doc;
        new_2d_doc.text("&New 2D Document");
        new_2d_doc.shortcut_keys(keys::control | keys::n);
        new_2d_doc.click += [this] {
            message_box::show("New 2D Document created!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)", 
                             "CAD System", 
                             message_box_buttons::ok, 
                             message_box_icon::information);
            status_label.text("New 2D Document created");
        };
        
        // Open Document menu item
        menu_item open_doc;
        open_doc.text("&Open...");
        open_doc.shortcut_keys(keys::control | keys::o);
        open_doc.click += [this] {
            open_file_dialog dialog;
            dialog.filter("CAD Documents (*.json)|*.json|All Files (*.*)|*.*");
            if (dialog.show_dialog(*this) == dialog_result::ok) {
                message_box::show("Opening: " + dialog.file_name(), "CAD System");
                status_label.text("Document opened: " + dialog.file_name());
            }
        };
        
        // Save Document menu item
        menu_item save_doc;
        save_doc.text("&Save");
        save_doc.shortcut_keys(keys::control | keys::s);
        save_doc.click += [this] {
            message_box::show("Document saved!", "CAD System");
            status_label.text("Document saved");
        };
        
        // Save As menu item
        menu_item save_as_doc;
        save_as_doc.text("Save &As...");
        save_as_doc.shortcut_keys(keys::control | keys::shift | keys::s);
        
        // Close menu item
        menu_item close_doc;
        close_doc.text("&Close");
        close_doc.shortcut_keys(keys::control | keys::w);
        
        // Separator
        menu_item separator1;
        separator1.text("-");
        
        // Exit menu item
        menu_item exit_item;
        exit_item.text("E&xit");
        exit_item.shortcut_keys(keys::control | keys::q);
        exit_item.click += [] {
            application::exit();
        };
        
        file_menu.menu_items().add(new_2d_doc);
        file_menu.menu_items().add(open_doc);
        file_menu.menu_items().add(save_doc);
        file_menu.menu_items().add(save_as_doc);
        file_menu.menu_items().add(close_doc);
        file_menu.menu_items().add(separator1);
        file_menu.menu_items().add(exit_item);
        
        // Edit Menu
        menu_item edit_menu;
        edit_menu.text("&Edit");
        
        menu_item undo_item;
        undo_item.text("&Undo");
        undo_item.shortcut_keys(keys::control | keys::z);
        
        menu_item redo_item;
        redo_item.text("&Redo");
        redo_item.shortcut_keys(keys::control | keys::y);
        
        menu_item separator2;
        separator2.text("-");
        
        menu_item cut_item;
        cut_item.text("Cu&t");
        cut_item.shortcut_keys(keys::control | keys::x);
        
        menu_item copy_item;
        copy_item.text("&Copy");
        copy_item.shortcut_keys(keys::control | keys::c);
        
        menu_item paste_item;
        paste_item.text("&Paste");
        paste_item.shortcut_keys(keys::control | keys::v);
        
        edit_menu.menu_items().add(undo_item);
        edit_menu.menu_items().add(redo_item);
        edit_menu.menu_items().add(separator2);
        edit_menu.menu_items().add(cut_item);
        edit_menu.menu_items().add(copy_item);
        edit_menu.menu_items().add(paste_item);
        
        // View Menu
        menu_item view_menu;
        view_menu.text("&View");
        
        menu_item zoom_in_item;
        zoom_in_item.text("Zoom &In");
        
        menu_item zoom_out_item;
        zoom_out_item.text("Zoom &Out");
        
        menu_item fit_window_item;
        fit_window_item.text("&Fit to Window");
        
        view_menu.menu_items().add(zoom_in_item);
        view_menu.menu_items().add(zoom_out_item);
        view_menu.menu_items().add(fit_window_item);
        
        // Tools Menu
        menu_item tools_menu;
        tools_menu.text("&Tools");
        
        menu_item options_item;
        options_item.text("&Options...");
        
        menu_item terminal_item;
        terminal_item.text("&Terminal...");
        
        tools_menu.menu_items().add(options_item);
        tools_menu.menu_items().add(terminal_item);
        
        // Help Menu
        menu_item help_menu;
        help_menu.text("&Help");
        
        menu_item about_item;
        about_item.text("&About...");
        about_item.click += [] {
            message_box::show("CAD System v1.0\nDriver Solution CAD\n\nBuilt with xtd framework", 
                             "About CAD System", 
                             message_box_buttons::ok, 
                             message_box_icon::information);
        };
        
        menu_item docs_item;
        docs_item.text("&Documentation");
        
        help_menu.menu_items().add(about_item);
        help_menu.menu_items().add(docs_item);
        
        // Add menus to form menu
        menu_items().add(file_menu);
        menu_items().add(edit_menu);
        menu_items().add(view_menu);
        menu_items().add(tools_menu);
        menu_items().add(help_menu);
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
    application::run(main_form {});
    return 0;
}

