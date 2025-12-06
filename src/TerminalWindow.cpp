#include "../include/TerminalWindow.h"
#include "../include/Solution.h"
#include <xtd/xtd>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

TerminalWindow::TerminalWindow(Solution* solution, const std::string& title, int width, int height)
    : solution_(solution), window_(nullptr), history_index_(-1) {
    prompt_ = "CAD> ";
    setupUI();
}

TerminalWindow::~TerminalWindow() {
}

void TerminalWindow::setupUI() {
    if (!solution_ || !solution_->getXTD()) {
        return;
    }
    
    window_ = solution_->getXTD()->createWindow("Terminal", 800, 600);
    auto& form = window_->getForm();
    
    // Output text box (read-only, multiline)
    output_text_box_.multiline(true);
    output_text_box_.read_only(true);
    output_text_box_.scroll_bars(xtd::forms::scroll_bars::vertical);
    output_text_box_.dock(xtd::forms::dock_style::fill);
    output_text_box_.parent(form);
    output_text_box_.font(xtd::drawing::font("Consolas", 10));
    output_text_box_.back_color(xtd::drawing::color::from_argb(30, 30, 30));
    output_text_box_.fore_color(xtd::drawing::color::from_argb(200, 200, 200));
    
    // Prompt label
    prompt_label_.text(prompt_);
    prompt_label_.location({10, 10});
    prompt_label_.size({50, 20});
    prompt_label_.parent(form);
    
    // Input text box
    input_text_box_.location({60, 8});
    input_text_box_.size({600, 25});
    input_text_box_.parent(form);
    input_text_box_.anchor(xtd::forms::anchor_styles::left | xtd::forms::anchor_styles::right | xtd::forms::anchor_styles::top);
    input_text_box_.key_down += [this](xtd::forms::control& sender, xtd::forms::key_event_args& e) {
        onInputKeyDown(sender, e);
    };
    
    // Execute button
    execute_button_.text("Execute");
    execute_button_.location({670, 8});
    execute_button_.size({100, 25});
    execute_button_.parent(form);
    execute_button_.anchor(xtd::forms::anchor_styles::right | xtd::forms::anchor_styles::top);
    execute_button_.click += [this] {
        onCommandEntered();
    };
    
    // Layout: output area at top, input at bottom
    output_text_box_.location({10, 40});
    output_text_box_.size({780, 520});
    output_text_box_.anchor(xtd::forms::anchor_styles::left | xtd::forms::anchor_styles::right | 
                           xtd::forms::anchor_styles::top | xtd::forms::anchor_styles::bottom);
    
    writeLine("CAD Terminal initialized");
    writeLine("Type 'help' for available commands");
    writeLine("");
}

void TerminalWindow::show() {
    if (window_) {
        window_->show();
    }
}

void TerminalWindow::hide() {
    if (window_) {
        window_->hide();
    }
}

bool TerminalWindow::isVisible() const {
    return window_ && window_->isVisible();
}

void TerminalWindow::write(const std::string& text) {
    appendOutputText(text);
}

void TerminalWindow::writeLine(const std::string& line) {
    appendOutputText(line + "\n");
}

void TerminalWindow::writeError(const std::string& error) {
    appendOutputText("[ERROR] " + error + "\n");
}

void TerminalWindow::writeInfo(const std::string& info) {
    appendOutputText("[INFO] " + info + "\n");
}

void TerminalWindow::writeWarning(const std::string& warning) {
    appendOutputText("[WARNING] " + warning + "\n");
}

void TerminalWindow::writeCommand(const std::string& command) {
    appendOutputText(prompt_ + command + "\n");
}

void TerminalWindow::writeResult(const std::string& result) {
    appendOutputText(result);
}

void TerminalWindow::clear() {
    output_text_box_.clear();
}

void TerminalWindow::setPrompt(const std::string& prompt) {
    prompt_ = prompt;
    if (window_) {
        prompt_label_.text(prompt_);
    }
}

void TerminalWindow::setCommandHandler(std::function<std::string(const std::string&)> handler) {
    command_handler_ = handler;
}

void TerminalWindow::executeCommand(const std::string& command) {
    if (command.empty()) {
        return;
    }
    
    // Add to history
    if (command_history_.empty() || command_history_.back() != command) {
        command_history_.push_back(command);
        if (command_history_.size() > MAX_HISTORY_SIZE) {
            command_history_.pop_front();
        }
    }
    history_index_ = command_history_.size();
    
    // Display command
    writeCommand(command);
    
    // Execute command
    std::string result;
    if (command_handler_) {
        result = command_handler_(command);
    } else {
        result = "No command handler set.\n";
    }
    
    // Display result
    if (!result.empty()) {
        writeResult(result);
    }
    
    // Clear input
    input_text_box_.clear();
    scrollToBottom();
}

void TerminalWindow::navigateHistoryUp() {
    if (command_history_.empty()) return;
    
    if (history_index_ > 0) {
        --history_index_;
    }
    
    if (history_index_ >= 0 && history_index_ < command_history_.size()) {
        input_text_box_.text(command_history_[history_index_]);
    }
}

void TerminalWindow::navigateHistoryDown() {
    if (command_history_.empty()) return;
    
    if (history_index_ < command_history_.size() - 1) {
        ++history_index_;
        input_text_box_.text(command_history_[history_index_]);
    } else {
        history_index_ = command_history_.size();
        input_text_box_.clear();
    }
}

void TerminalWindow::update() {
    // Update can be called periodically if needed
}

void TerminalWindow::onCommandEntered() {
    std::string command = input_text_box_.text();
    executeCommand(command);
}

void TerminalWindow::onInputKeyDown(xtd::forms::control& sender, xtd::forms::key_event_args& e) {
    if (e.key_code() == xtd::forms::keys::enter) {
        onCommandEntered();
        e.handled(true);
    } else if (e.key_code() == xtd::forms::keys::up) {
        navigateHistoryUp();
        e.handled(true);
    } else if (e.key_code() == xtd::forms::keys::down) {
        navigateHistoryDown();
        e.handled(true);
    }
}

void TerminalWindow::appendOutputText(const std::string& text) {
    output_text_box_.append_text(text);
    trimOutputIfNeeded();
    scrollToBottom();
}

void TerminalWindow::scrollToBottom() {
    output_text_box_.selection_start(output_text_box_.text().length());
    output_text_box_.scroll_to_caret();
}

std::string TerminalWindow::getTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

void TerminalWindow::trimOutputIfNeeded() {
    std::string text = output_text_box_.text();
    size_t line_count = std::count(text.begin(), text.end(), '\n');
    
    if (line_count > MAX_OUTPUT_LINES) {
        size_t lines_to_remove = line_count - MAX_OUTPUT_LINES + TRIM_LINES_COUNT;
        size_t pos = 0;
        for (size_t i = 0; i < lines_to_remove && pos < text.length(); ++i) {
            pos = text.find('\n', pos);
            if (pos != std::string::npos) {
                ++pos;
            } else {
                break;
            }
        }
        if (pos < text.length()) {
            output_text_box_.text(text.substr(pos));
        } else {
            output_text_box_.clear();
        }
    }
}

