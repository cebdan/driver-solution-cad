#include "../include/Solution.h"
#include <algorithm>
#include <sstream>
#include <cctype>

Solution::Solution() : name_("Solution") {
}

Solution::~Solution() {
}

void Solution::addConstructionStep(const std::string& operation, void* data) {
    construction_history_.addStep(operation, data);
}

void Solution::undoConstruction() {
    construction_history_.undo();
}

void Solution::redoConstruction() {
    construction_history_.redo();
}

void Solution::addNode(std::unique_ptr<Node> node) {
    nodes_.push_back(std::move(node));
}

Node* Solution::getNode(const std::string& name) const {
    for (const auto& node : nodes_) {
        if (node->getName() == name) {
            return node.get();
        }
    }
    return nullptr;
}

void Solution::removeNode(const std::string& name) {
    nodes_.erase(
        std::remove_if(nodes_.begin(), nodes_.end(),
            [&name](const std::unique_ptr<Node>& node) {
                return node->getName() == name;
            }),
        nodes_.end()
    );
}

std::vector<Node*> Solution::getAllNodes() const {
    std::vector<Node*> result;
    for (const auto& node : nodes_) {
        result.push_back(node.get());
    }
    return result;
}

void Solution::executeNode(const std::string& name) {
    Node* node = getNode(name);
    if (node) {
        node->execute();
    }
}

void Solution::executeAllNodes() {
    for (const auto& node : nodes_) {
        node->execute();
    }
}

void Solution::initializeXTD() {
    if (!xtd_) {
        xtd_ = std::make_unique<XTD>();
        xtd_->setSolution(this);
        xtd_->initialize();
    }
}

void Solution::updateXTD() {
    if (xtd_ && xtd_->isInitialized()) {
        xtd_->update();
    }
}

void Solution::renderXTD() {
    if (xtd_ && xtd_->isInitialized()) {
        xtd_->update();
    }
}

XTDWindow* Solution::createXTDWindow(const std::string& title, int width, int height) {
    initializeXTD();
    return xtd_->createWindow(title, width, height);
}

TerminalWindow* Solution::createTerminal(const std::string& title, int width, int height) {
    if (!terminal_) {
        initializeXTD();
        terminal_ = std::make_unique<TerminalWindow>(this, title, width, height);
        terminal_->setCommandHandler([this](const std::string& cmd) {
            return processTerminalCommand(cmd);
        });
    }
    return terminal_.get();
}

void Solution::showTerminal() {
    if (terminal_) {
        terminal_->show();
    }
}

void Solution::hideTerminal() {
    if (terminal_) {
        terminal_->hide();
    }
}

void Solution::initializeRenderer() {
    if (!renderer_) {
        renderer_ = std::make_unique<OpenGLRenderer>();
        renderer_->initialize();
    }
}

void Solution::render() {
    if (renderer_ && renderer_->isInitialized()) {
        renderer_->beginRender();
        renderer_->clear(0.2f, 0.2f, 0.2f, 1.0f);
        renderer_->endRender();
    }
}

bool Solution::canReceiveData(const std::string& data_type) const {
    return canProcessDataType(data_type);
}

bool Solution::canSendData(const std::string& data_type) const {
    return canProcessDataType(data_type);
}

void* Solution::receiveData(void* data, const std::string& data_type) {
    return processIncomingData(data, data_type);
}

void* Solution::sendData(const std::string& data_type) {
    return prepareOutgoingData(data_type);
}

bool Solution::canExchangeDataWith(Solution* other_solution, const std::string& data_type) {
    if (!other_solution) return false;
    return canSendData(data_type) && other_solution->canReceiveData(data_type);
}

void* Solution::exchangeDataWith(Solution* other_solution, const std::string& data_type, void* data) {
    if (!other_solution || !canExchangeDataWith(other_solution, data_type)) {
        return nullptr;
    }
    void* processed = sendData(data_type);
    return other_solution->receiveData(processed, data_type);
}

bool Solution::canProcessDataType(const std::string& data_type) const {
    return false; // Override in derived classes
}

void* Solution::processIncomingData(void* data, const std::string& data_type) {
    return data; // Override in derived classes
}

void* Solution::prepareOutgoingData(const std::string& data_type) {
    return nullptr; // Override in derived classes
}

std::string Solution::processTerminalCommand(const std::string& command) {
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
    } else if (cmd == "status") {
        result << "Solution: " << name_ << "\n";
        result << "Nodes: " << nodes_.size() << "\n";
        result << "History steps: " << construction_history_.getStepCount() << "\n";
        result << "XTD initialized: " << (xtd_ && xtd_->isInitialized() ? "yes" : "no") << "\n";
        result << "Renderer initialized: " << (renderer_ && renderer_->isInitialized() ? "yes" : "no") << "\n";
    } else if (cmd == "nodes") {
        if (nodes_.empty()) {
            result << "No nodes defined.\n";
        } else {
            result << "Nodes (" << nodes_.size() << "):\n";
            for (const auto& node : nodes_) {
                result << "  - " << node->getName() << " (" << node->getType() << ")\n";
            }
        }
    } else if (cmd == "history") {
        size_t count = construction_history_.getStepCount();
        if (count == 0) {
            result << "No construction history.\n";
        } else {
            result << "Construction history (" << count << " steps):\n";
            for (size_t i = 0; i < count; ++i) {
                auto* step = construction_history_.getStep(i);
                if (step) {
                    result << "  " << (i + 1) << ". " << step->getOperation() << "\n";
                }
            }
        }
    } else if (cmd == "name") {
        result << "Solution name: " << name_ << "\n";
    } else if (cmd.empty()) {
        // Empty command, do nothing
    } else {
        result << "Unknown command: " << command << "\n";
        result << "Type 'help' for available commands.\n";
    }
    
    return result.str();
}

