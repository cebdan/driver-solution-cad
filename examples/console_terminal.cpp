#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <sstream>
#include <cctype>

class ConsoleTerminal {
public:
    ConsoleTerminal() : history_index_(-1) {
        prompt_ = "CAD> ";
    }
    
    void writeLine(const std::string& line = "") {
        std::cout << line << std::endl;
    }
    
    void writeInfo(const std::string& info) {
        std::cout << "[INFO] " << info << std::endl;
    }
    
    void writeError(const std::string& error) {
        std::cout << "[ERROR] " << error << std::endl;
    }
    
    void writeWarning(const std::string& warning) {
        std::cout << "[WARNING] " << warning << std::endl;
    }
    
    void writeCommand(const std::string& command) {
        std::cout << prompt_ << command << std::endl;
    }
    
    void writeResult(const std::string& result) {
        std::cout << result;
    }
    
    std::string readCommand() {
        std::cout << prompt_;
        std::string command;
        std::getline(std::cin, command);
        return command;
    }
    
    void addToHistory(const std::string& command) {
        if (!command.empty() && (command_history_.empty() || command_history_.back() != command)) {
            command_history_.push_back(command);
            if (command_history_.size() > 100) {
                command_history_.pop_front();
            }
        }
        history_index_ = command_history_.size();
    }
    
    void clear() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
private:
    std::string prompt_;
    std::deque<std::string> command_history_;
    int history_index_;
};

std::string processCommand(const std::string& command, const std::string& solution_name) {
    std::stringstream result;
    std::string cmd = command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd == "help" || cmd == "?") {
        result << "Available commands:\n";
        result << "  help, ? - Show this help\n";
        result << "  status - Show solution status\n";
        result << "  nodes - List all nodes\n";
        result << "  history - Show construction history\n";
        result << "  clear - Clear terminal\n";
        result << "  name - Show solution name\n";
        result << "  exit, quit - Exit terminal\n";
    } else if (cmd == "status") {
        result << "Solution: " << solution_name << "\n";
        result << "Status: Running\n";
        result << "Terminal: Active\n";
    } else if (cmd == "nodes") {
        result << "Nodes: 0 (no nodes defined)\n";
    } else if (cmd == "history") {
        result << "Construction history: 0 steps\n";
    } else if (cmd == "name") {
        result << "Solution name: " << solution_name << "\n";
    } else if (cmd == "clear") {
        result << "\033[2J\033[1;1H"; // ANSI clear screen
        result << "Terminal cleared.\n";
    } else if (cmd == "exit" || cmd == "quit") {
        result << "Exiting...\n";
    } else if (cmd.empty()) {
        // Empty command
    } else {
        result << "Unknown command: " << command << "\n";
        result << "Type 'help' for available commands.\n";
    }
    
    return result.str();
}

int main() {
    ConsoleTerminal terminal;
    std::string solution_name = "Demo CAD Solution";
    
    terminal.clear();
    terminal.writeLine("=== CAD System Terminal ===");
    terminal.writeLine("");
    terminal.writeInfo("System initialized successfully");
    terminal.writeLine("Type 'help' to see available commands");
    terminal.writeLine("Type 'exit' or 'quit' to exit");
    terminal.writeLine("");
    
    bool running = true;
    
    while (running) {
        std::string command = terminal.readCommand();
        
        if (command.empty()) {
            continue;
        }
        
        terminal.addToHistory(command);
        
        std::string cmd_lower = command;
        std::transform(cmd_lower.begin(), cmd_lower.end(), cmd_lower.begin(), ::tolower);
        
        if (cmd_lower == "exit" || cmd_lower == "quit") {
            running = false;
            terminal.writeLine("Goodbye!");
            break;
        }
        
        if (cmd_lower == "clear") {
            terminal.clear();
            terminal.writeLine("=== CAD System Terminal ===");
            terminal.writeLine("");
            continue;
        }
        
        terminal.writeCommand(command);
        std::string result = processCommand(command, solution_name);
        terminal.writeResult(result);
        terminal.writeLine("");
    }
    
    return 0;
}

