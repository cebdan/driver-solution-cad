#include "../include/XTD.h"
#include "../include/Solution.h"
#include <xtd/xtd>

// XTDWidget
XTDWidget::XTDWidget(const std::string& name, const std::string& type)
    : name_(name), type_(type), x_(0), y_(0), width_(100), height_(30), visible_(true) {
}

void XTDWidget::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
    getControl().location({x, y});
}

void XTDWidget::setSize(int width, int height) {
    width_ = width;
    height_ = height;
    getControl().size({width, height});
}

void XTDWidget::setVisible(bool visible) {
    visible_ = visible;
    getControl().visible(visible);
}

bool XTDWidget::isVisible() const {
    return getControl().visible();
}

// XTDButton
XTDButton::XTDButton(const std::string& name, const std::string& label, std::function<void()> callback)
    : XTDWidget(name, "button") {
    button_.text(label);
    if (callback) {
        button_.click += callback;
    }
}

void XTDButton::setText(const std::string& text) {
    button_.text(text);
}

std::string XTDButton::getText() const {
    return button_.text();
}

// XTDLabel
XTDLabel::XTDLabel(const std::string& name, const std::string& text)
    : XTDWidget(name, "label") {
    label_.text(text);
}

void XTDLabel::setText(const std::string& text) {
    label_.text(text);
}

std::string XTDLabel::getText() const {
    return label_.text();
}

// XTDInput
XTDInput::XTDInput(const std::string& name, const std::string& default_value)
    : XTDWidget(name, "input") {
    text_box_.text(default_value);
}

void XTDInput::setText(const std::string& text) {
    text_box_.text(text);
}

std::string XTDInput::getText() const {
    return text_box_.text();
}

void XTDInput::setMultiline(bool multiline) {
    text_box_.multiline(multiline);
}

void XTDInput::setReadOnly(bool read_only) {
    text_box_.read_only(read_only);
}

void XTDInput::appendText(const std::string& text) {
    text_box_.append_text(text);
}

void XTDInput::clear() {
    text_box_.clear();
}

// XTDSlider
XTDSlider::XTDSlider(const std::string& name, double min_value, double max_value, double default_value)
    : XTDWidget(name, "slider") {
    track_bar_.minimum(static_cast<int>(min_value));
    track_bar_.maximum(static_cast<int>(max_value));
    track_bar_.value(static_cast<int>(default_value));
}

void XTDSlider::setValue(double value) {
    track_bar_.value(static_cast<int>(value));
}

double XTDSlider::getValue() const {
    return static_cast<double>(track_bar_.value());
}

void XTDSlider::setRange(double min_value, double max_value) {
    track_bar_.minimum(static_cast<int>(min_value));
    track_bar_.maximum(static_cast<int>(max_value));
}

// XTDCheckbox
XTDCheckbox::XTDCheckbox(const std::string& name, const std::string& label, bool default_value)
    : XTDWidget(name, "checkbox") {
    check_box_.text(label);
    check_box_.checked(default_value);
}

void XTDCheckbox::setChecked(bool checked) {
    check_box_.checked(checked);
}

bool XTDCheckbox::isChecked() const {
    return check_box_.checked();
}

// XTDComboBox
XTDComboBox::XTDComboBox(const std::string& name, const std::vector<std::string>& options)
    : XTDWidget(name, "combobox") {
    for (const auto& option : options) {
        combo_box_.items().push_back(option);
    }
}

void XTDComboBox::addItem(const std::string& item) {
    combo_box_.items().push_back(item);
}

void XTDComboBox::removeItem(const std::string& item) {
    auto& items = combo_box_.items();
    items.erase(std::remove(items.begin(), items.end(), item), items.end());
}

std::string XTDComboBox::getSelectedItem() const {
    if (combo_box_.selected_index() >= 0 && combo_box_.selected_index() < combo_box_.items().size()) {
        return combo_box_.items()[combo_box_.selected_index()];
    }
    return "";
}

void XTDComboBox::setSelectedItem(const std::string& item) {
    auto& items = combo_box_.items();
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i] == item) {
            combo_box_.selected_index(static_cast<int>(i));
            return;
        }
    }
}

// XTDWindow
XTDWindow::XTDWindow(const std::string& title, int width, int height)
    : title_(title), width_(width), height_(height), visible_(false) {
    form_.text(title);
    form_.size({width, height});
}

XTDWindow::~XTDWindow() {
}

void XTDWindow::addWidget(std::unique_ptr<XTDWidget> widget) {
    if (widget) {
        widgets_[widget->getName()] = std::move(widget);
    }
}

void XTDWindow::removeWidget(const std::string& widget_name) {
    widgets_.erase(widget_name);
}

XTDWidget* XTDWindow::getWidget(const std::string& name) const {
    auto it = widgets_.find(name);
    if (it != widgets_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void XTDWindow::setTitle(const std::string& title) {
    title_ = title;
    form_.text(title);
}

std::string XTDWindow::getTitle() const {
    return form_.text();
}

void XTDWindow::show() {
    visible_ = true;
    form_.show();
}

void XTDWindow::hide() {
    visible_ = false;
    form_.hide();
}

bool XTDWindow::isVisible() const {
    return form_.visible();
}

// XTD
XTD::XTD() : initialized_(false), solution_(nullptr) {
}

XTD::~XTD() {
    shutdown();
}

bool XTD::initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    return true;
}

void XTD::shutdown() {
    windows_.clear();
    initialized_ = false;
}

void XTD::update() {
    if (initialized_) {
        xtd::forms::application::do_events();
    }
}

void XTD::run() {
    if (initialized_) {
        xtd::forms::application::run();
    }
}

XTDWindow* XTD::createWindow(const std::string& title, int width, int height) {
    auto window = std::make_unique<XTDWindow>(title, width, height);
    XTDWindow* ptr = window.get();
    windows_.push_back(std::move(window));
    return ptr;
}

void XTD::destroyWindow(XTDWindow* window) {
    windows_.erase(
        std::remove_if(windows_.begin(), windows_.end(),
            [window](const std::unique_ptr<XTDWindow>& w) {
                return w.get() == window;
            }),
        windows_.end()
    );
}

XTDButton* XTD::createButton(const std::string& name, const std::string& label, std::function<void()> callback) {
    // This would need to be added to a window
    return nullptr; // Placeholder
}

XTDLabel* XTD::createLabel(const std::string& name, const std::string& text) {
    return nullptr; // Placeholder
}

XTDInput* XTD::createInput(const std::string& name, const std::string& default_value) {
    return nullptr; // Placeholder
}

XTDSlider* XTD::createSlider(const std::string& name, double min_value, double max_value, double default_value) {
    return nullptr; // Placeholder
}

XTDCheckbox* XTD::createCheckbox(const std::string& name, const std::string& label, bool default_value) {
    return nullptr; // Placeholder
}

XTDComboBox* XTD::createComboBox(const std::string& name, const std::vector<std::string>& options) {
    return nullptr; // Placeholder
}

void XTD::processEvents() {
    update();
}

void XTD::showWindow(XTDWindow* window) {
    if (window) {
        window->show();
    }
}

void XTD::hideWindow(XTDWindow* window) {
    if (window) {
        window->hide();
    }
}

void XTD::messageBox(const std::string& message, const std::string& title) {
    xtd::forms::message_box::show(message, title);
}

