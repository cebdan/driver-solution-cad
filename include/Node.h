#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <memory>
#include <map>

class Node {
public:
    Node(const std::string& name, const std::string& type);
    virtual ~Node() = default;
    
    std::string getName() const { return name_; }
    std::string getType() const { return type_; }
    
    void addInput(const std::string& name, void* data);
    void addOutput(const std::string& name, void* data);
    void* getInput(const std::string& name) const;
    void* getOutput(const std::string& name) const;
    
    void connectTo(Node* target_node, const std::string& output_name, const std::string& input_name);
    std::vector<Node*> getConnectedNodes() const;
    
    virtual void execute() = 0;
    
protected:
    std::string name_;
    std::string type_;
    std::map<std::string, void*> inputs_;
    std::map<std::string, void*> outputs_;
    std::vector<Node*> connected_nodes_;
};

class ScriptNode : public Node {
public:
    ScriptNode(const std::string& name);
    virtual ~ScriptNode() = default;
    
    void setScript(const std::string& script);
    std::string getScript() const { return script_; }
    
    virtual void execute() override;
    
private:
    std::string script_;
};

#endif // NODE_H

