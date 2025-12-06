#include "../include/MainWindow.h"
#include "../include/XTD.h"
#include "../include/Document2D.h"
#include "../include/CS.h"
#include <xtd/xtd>

MainWindow::MainWindow() : window_(nullptr), visible_(false), title_("CAD System") {
    setupUI();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // Create main form
    main_form_ = new xtd::forms::form();
    main_form_->text(title_);
    main_form_->size({800, 600});
    main_form_->start_position(xtd::forms::form_start_position::center_screen);
    
    // Setup menu bar
    setupMenuBar();
}

void MainWindow::show() {
    if (main_form_) {
        main_form_->show();
        visible_ = true;
    }
}

void MainWindow::hide() {
    if (main_form_) {
        main_form_->hide();
        visible_ = false;
    }
}

bool MainWindow::isVisible() const {
    return main_form_ && main_form_->visible();
}

void MainWindow::setTitle(const std::string& title) {
    title_ = title;
    if (main_form_) {
        main_form_->text(title);
    }
}

std::string MainWindow::getTitle() const {
    return title_;
}

void MainWindow::createMenuBar() {
    setupMenuBar();
}

void MainWindow::setupMenuBar() {
    if (!main_form_) return;
    
    // Create menu bar
    menu_bar_ = new xtd::forms::menu_bar();
    
    // File Menu
    file_menu_ = new xtd::forms::menu_item();
    file_menu_->text("&File");
    
    // New 2D Document menu item
    new_2d_doc_item_ = new xtd::forms::menu_item();
    new_2d_doc_item_->text("&New 2D Document");
    new_2d_doc_item_->shortcut_keys(xtd::forms::keys::control | xtd::forms::keys::n);
    new_2d_doc_item_->click += [this](xtd::object& sender, const xtd::event_args& e) {
        onNew2DDocument();
    };
    
    // Open Document menu item
    open_item_ = new xtd::forms::menu_item();
    open_item_->text("&Open...");
    open_item_->shortcut_keys(xtd::forms::keys::control | xtd::forms::keys::o);
    open_item_->click += [this](xtd::object& sender, const xtd::event_args& e) {
        onOpenDocument();
    };
    
    // Save Document menu item
    save_item_ = new xtd::forms::menu_item();
    save_item_->text("&Save");
    save_item_->shortcut_keys(xtd::forms::keys::control | xtd::forms::keys::s);
    save_item_->click += [this](xtd::object& sender, const xtd::event_args& e) {
        onSaveDocument();
    };
    
    // Save As menu item
    auto save_as_item = xtd::forms::menu_item::create("Save &As...", 
        {xtd::forms::keys::control | xtd::forms::keys::shift | xtd::forms::keys::s});
    
    // Close menu item
    auto close_item = xtd::forms::menu_item::create("&Close", 
        {xtd::forms::keys::control | xtd::forms::keys::w});
    close_item->click += [this](xtd::object& sender, const xtd::event_args& e) {
        onCloseDocument();
    };
    
    // Separator
    auto separator = xtd::forms::menu_item::create_separator();
    
    // Exit menu item
    exit_item_ = new xtd::forms::menu_item();
    exit_item_->text("E&xit");
    exit_item_->shortcut_keys(xtd::forms::keys::control | xtd::forms::keys::q);
    exit_item_->click += [this](xtd::object& sender, const xtd::event_args& e) {
        onExit();
    };
    
    // Add items to File menu
    file_menu_->menu_items().push_back(*new_2d_doc_item_);
    file_menu_->menu_items().push_back(*open_item_);
    file_menu_->menu_items().push_back(*save_item_);
    file_menu_->menu_items().push_back(save_as_item);
    file_menu_->menu_items().push_back(close_item);
    file_menu_->menu_items().push_back(separator);
    file_menu_->menu_items().push_back(*exit_item_);
    
    // Add File menu to menu bar
    menu_bar_->menu_items().push_back(*file_menu_);
    
    // Set menu bar to form
    main_form_->menu(*menu_bar_);
}

void MainWindow::addMenuItem(const std::string& menu_name, const std::string& item_name, std::function<void()> callback) {
    // Add menu item implementation
    // This would use xtd::forms::menu_item
}

void MainWindow::createNew2DDocument() {
    onNew2DDocument();
}

void MainWindow::onNew2DDocument() {
    // Create new 2D document
    current_document_ = std::make_unique<Document2D>("New 2D Document");
    
    // Set default coordinate system
    static CS globalCS = CS::GlobalCS();
    current_document_->setDefaultCoordinateSystem(&globalCS);
    
    // Call callback if set
    if (on_new_2d_document_) {
        on_new_2d_document_();
    }
}

void MainWindow::openDocument() {
    onOpenDocument();
}

void MainWindow::onOpenDocument() {
    // Open document dialog would be implemented here
    // For now, just a placeholder
    if (on_open_document_) {
        on_open_document_();
    }
}

void MainWindow::saveDocument() {
    onSaveDocument();
}

void MainWindow::onSaveDocument() {
    if (current_document_) {
        std::string path = current_document_->getPath();
        if (path.empty()) {
            // Show save dialog
            path = "document.json"; // Default path
        }
        current_document_->save(path);
    }
    
    if (on_save_document_) {
        on_save_document_();
    }
}

void MainWindow::closeDocument() {
    onCloseDocument();
}

void MainWindow::onCloseDocument() {
    if (current_document_) {
        current_document_->close();
        current_document_.reset();
    }
    
    if (on_close_document_) {
        on_close_document_();
    }
}

void MainWindow::onExit() {
    if (on_exit_) {
        on_exit_();
    } else {
        // Default exit behavior
        hide();
    }
}

void MainWindow::setOnNew2DDocument(std::function<void()> callback) {
    on_new_2d_document_ = callback;
}

void MainWindow::setOnOpenDocument(std::function<void()> callback) {
    on_open_document_ = callback;
}

void MainWindow::setOnSaveDocument(std::function<void()> callback) {
    on_save_document_ = callback;
}

void MainWindow::setOnCloseDocument(std::function<void()> callback) {
    on_close_document_ = callback;
}

void MainWindow::setOnExit(std::function<void()> callback) {
    on_exit_ = callback;
}

void MainWindow::update() {
    // Update window if needed
}

