#ifndef XTD_H
#define XTD_H

#include <xtd/xtd>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

class Solution;

// Wrapper classes for xtd framework integration
class XTDWidget {
public:
    XTDWidget(const std::string& name, const std::string& type);
    virtual ~XTDWidget() = default;
    
    std::string getName() const { return name_; }
    std::string getType() const { return type_; }
    
    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setVisible(bool visible);
    bool isVisible() const;
    
    virtual xtd::forms::control& getControl() = 0;
    
protected:
    std::string name_;
    std::string type_;
    int x_, y_;
    int width_, height_;
    bool visible_;
};

class XTDButton : public XTDWidget {
public:
    XTDButton(const std::string& name, const std::string& label, std::function<void()> callback);
    virtual ~XTDButton() = default;
    
    virtual xtd::forms::control& getControl() override { return button_; }
    void setText(const std::string& text);
    std::string getText() const;
    
private:
    xtd::forms::button button_;
};

class XTDLabel : public XTDWidget {
public:
    XTDLabel(const std::string& name, const std::string& text);
    virtual ~XTDLabel() = default;
    
    virtual xtd::forms::control& getControl() override { return label_; }
    void setText(const std::string& text);
    std::string getText() const;
    
private:
    xtd::forms::label label_;
};

class XTDInput : public XTDWidget {
public:
    XTDInput(const std::string& name, const std::string& default_value = "");
    virtual ~XTDInput() = default;
    
    virtual xtd::forms::control& getControl() override { return text_box_; }
    void setText(const std::string& text);
    std::string getText() const;
    void setMultiline(bool multiline);
    void setReadOnly(bool read_only);
    void appendText(const std::string& text);
    void clear();
    
private:
    xtd::forms::text_box text_box_;
};

class XTDSlider : public XTDWidget {
public:
    XTDSlider(const std::string& name, double min_value, double max_value, double default_value);
    virtual ~XTDSlider() = default;
    
    virtual xtd::forms::control& getControl() override { return track_bar_; }
    void setValue(double value);
    double getValue() const;
    void setRange(double min_value, double max_value);
    
private:
    xtd::forms::track_bar track_bar_;
};

class XTDCheckbox : public XTDWidget {
public:
    XTDCheckbox(const std::string& name, const std::string& label, bool default_value = false);
    virtual ~XTDCheckbox() = default;
    
    virtual xtd::forms::control& getControl() override { return check_box_; }
    void setChecked(bool checked);
    bool isChecked() const;
    
private:
    xtd::forms::check_box check_box_;
};

class XTDComboBox : public XTDWidget {
public:
    XTDComboBox(const std::string& name, const std::vector<std::string>& options);
    virtual ~XTDComboBox() = default;
    
    virtual xtd::forms::control& getControl() override { return combo_box_; }
    void addItem(const std::string& item);
    void removeItem(const std::string& item);
    std::string getSelectedItem() const;
    void setSelectedItem(const std::string& item);
    
private:
    xtd::forms::combo_box combo_box_;
};

class XTDWindow {
public:
    XTDWindow(const std::string& title, int width, int height);
    virtual ~XTDWindow();
    
    void addWidget(std::unique_ptr<XTDWidget> widget);
    void removeWidget(const std::string& widget_name);
    XTDWidget* getWidget(const std::string& name) const;
    
    void setTitle(const std::string& title);
    std::string getTitle() const;
    
    void show();
    void hide();
    bool isVisible() const;
    
    xtd::forms::form& getForm() { return form_; }
    
private:
    xtd::forms::form form_;
    std::map<std::string, std::unique_ptr<XTDWidget>> widgets_;
};

class XTD {
public:
    XTD();
    virtual ~XTD();
    
    bool initialize();
    void shutdown();
    void update();
    void run();
    
    XTDWindow* createWindow(const std::string& title, int width, int height);
    void destroyWindow(XTDWindow* window);
    
    XTDButton* createButton(const std::string& name, const std::string& label, std::function<void()> callback);
    XTDLabel* createLabel(const std::string& name, const std::string& text);
    XTDInput* createInput(const std::string& name, const std::string& default_value = "");
    XTDSlider* createSlider(const std::string& name, double min_value, double max_value, double default_value);
    XTDCheckbox* createCheckbox(const std::string& name, const std::string& label, bool default_value = false);
    XTDComboBox* createComboBox(const std::string& name, const std::vector<std::string>& options);
    
    void setSolution(Solution* solution) { solution_ = solution; }
    Solution* getSolution() const { return solution_; }
    
    bool isInitialized() const { return initialized_; }
    
    void processEvents();
    void showWindow(XTDWindow* window);
    void hideWindow(XTDWindow* window);
    
    static void messageBox(const std::string& message, const std::string& title = "");
    
private:
    bool initialized_;
    Solution* solution_;
    std::vector<std::unique_ptr<XTDWindow>> windows_;
};

#endif // XTD_H

