#include "../include/Node.h"

Node::Node(const std::string& name, const std::string& type)
    : name_(name), type_(type) {
}

void Node::addInput(const std::string& name, void* data) {
    inputs_[name] = data;
}

void Node::addOutput(const std::string& name, void* data) {
    outputs_[name] = data;
}

void* Node::getInput(const std::string& name) const {
    auto it = inputs_.find(name);
    if (it != inputs_.end()) {
        return it->second;
    }
    return nullptr;
}

void* Node::getOutput(const std::string& name) const {
    auto it = outputs_.find(name);
    if (it != outputs_.end()) {
        return it->second;
    }
    return nullptr;
}

void Node::connectTo(Node* target_node, const std::string& output_name, const std::string& input_name) {
    if (target_node) {
        void* output_data = getOutput(output_name);
        if (output_data) {
            target_node->addInput(input_name, output_data);
            connected_nodes_.push_back(target_node);
        }
    }
}

std::vector<Node*> Node::getConnectedNodes() const {
    return connected_nodes_;
}

ScriptNode::ScriptNode(const std::string& name)
    : Node(name, "script") {
}

void ScriptNode::setScript(const std::string& script) {
    script_ = script;
}

void ScriptNode::execute() {
    // Script execution would be implemented here
    // This is a placeholder
}

