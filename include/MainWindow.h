#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "XTD.h"
#include "Document2D.h"
#include <xtd/xtd>
#include <string>
#include <vector>
#include <memory>
#include <functional>

class MainWindow {
public:
    MainWindow();
    virtual ~MainWindow();
    
    void show();
    void hide();
    bool isVisible() const;
    
    void setTitle(const std::string& title);
    std::string getTitle() const;
    
    // Menu creation
    void createMenuBar();
    void addMenuItem(const std::string& menu_name, const std::string& item_name, std::function<void()> callback);
    
    // Document management
    void createNew2DDocument();
    void openDocument();
    void saveDocument();
    void closeDocument();
    
    // Event handlers
    void setOnNew2DDocument(std::function<void()> callback);
    void setOnOpenDocument(std::function<void()> callback);
    void setOnSaveDocument(std::function<void()> callback);
    void setOnCloseDocument(std::function<void()> callback);
    void setOnExit(std::function<void()> callback);
    
    // Get current document
    Document2D* getCurrentDocument() const { return current_document_.get(); }
    
    // Window access
    XTDWindow* getWindow() { return window_; }
    
    void update();
    
private:
    void setupUI();
    void setupMenuBar();
    void onNew2DDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onCloseDocument();
    void onExit();
    
    XTDWindow* window_;
    std::unique_ptr<Document2D> current_document_;
    
    // xtd GUI components
    xtd::forms::form* main_form_;
    xtd::forms::menu_bar* menu_bar_;
    xtd::forms::menu_item* file_menu_;
    xtd::forms::menu_item* new_2d_doc_item_;
    xtd::forms::menu_item* open_item_;
    xtd::forms::menu_item* save_item_;
    xtd::forms::menu_item* exit_item_;
    
    // Event callbacks
    std::function<void()> on_new_2d_document_;
    std::function<void()> on_open_document_;
    std::function<void()> on_save_document_;
    std::function<void()> on_close_document_;
    std::function<void()> on_exit_;
    
    std::string title_;
    bool visible_;
};

#endif // MAIN_WINDOW_H

