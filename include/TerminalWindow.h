#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include "XTD.h"
#include <xtd/xtd>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <deque>
#include <sstream>

class Solution;

class TerminalWindow {
public:
    TerminalWindow(Solution* solution, const std::string& title = "Terminal", int width = 800, int height = 600);
    virtual ~TerminalWindow();
    
    void show();
    void hide();
    bool isVisible() const;
    
    void write(const std::string& text);
    void writeLine(const std::string& line = "");
    void writeError(const std::string& error);
    void writeInfo(const std::string& info);
    void writeWarning(const std::string& warning);
    void writeCommand(const std::string& command);
    void writeResult(const std::string& result);
    
    void clear();
    void setPrompt(const std::string& prompt);
    std::string getPrompt() const { return prompt_; }
    
    void setCommandHandler(std::function<std::string(const std::string&)> handler);
    void executeCommand(const std::string& command);
    
    void navigateHistoryUp();
    void navigateHistoryDown();
    
    XTDWindow* getWindow() { return window_; }
    
    void update();
    
private:
    void setupUI();
    void onCommandEntered();
    void onInputKeyDown(xtd::forms::control& sender, xtd::forms::key_event_args& e);
    void appendOutputText(const std::string& text);
    void scrollToBottom();
    std::string getTimestamp() const;
    void trimOutputIfNeeded();
    
    Solution* solution_;
    XTDWindow* window_;
    xtd::forms::text_box output_text_box_;
    xtd::forms::text_box input_text_box_;
    xtd::forms::button execute_button_;
    xtd::forms::label prompt_label_;
    
    std::string prompt_;
    std::deque<std::string> command_history_;
    int history_index_;
    std::string current_input_;
    std::function<std::string(const std::string&)> command_handler_;
    
    static const size_t MAX_HISTORY_SIZE = 1000;
    static const size_t MAX_OUTPUT_LINES = 10000;
    static const size_t TRIM_LINES_COUNT = 1000;
};

#endif // TERMINAL_WINDOW_H

