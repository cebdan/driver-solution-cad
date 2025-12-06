#ifndef SOLUTION_H
#define SOLUTION_H

#include "ConstructionHistory.h"
#include "Node.h"
#include "XTD.h"
#include "DataExchange.h"
#include "OpenGLRenderer.h"
#include "TerminalWindow.h"
#include <vector>
#include <memory>
#include <string>

class Solution : public DataExchangeInterface {
public:
    Solution();
    virtual ~Solution();
    
    // Core microkernel methods
    virtual void solve() = 0;
    virtual void new_solution() = 0;
    virtual void delete_solution() = 0;
    virtual void copy() = 0;
    virtual void duplication() = 0;
    virtual void propagation() = 0;
    virtual void similar_make() = 0;
    
    // Construction history management
    ConstructionHistory* getConstructionHistory() { return &construction_history_; }
    void addConstructionStep(const std::string& operation, void* data = nullptr);
    void undoConstruction();
    void redoConstruction();
    
    // Node system for scripts
    void addNode(std::unique_ptr<Node> node);
    Node* getNode(const std::string& name) const;
    void removeNode(const std::string& name);
    std::vector<Node*> getAllNodes() const;
    void executeNode(const std::string& name);
    void executeAllNodes();
    
    // XTD GUI system
    XTD* getXTD() { return xtd_.get(); }
    void initializeXTD();
    void updateXTD();
    void renderXTD();
    XTDWindow* createXTDWindow(const std::string& title, int width, int height);
    
    // Terminal window
    TerminalWindow* getTerminal() { return terminal_.get(); }
    TerminalWindow* createTerminal(const std::string& title = "Terminal", int width = 800, int height = 600);
    void showTerminal();
    void hideTerminal();
    
    // OpenGL rendering
    OpenGLRenderer* getRenderer() { return renderer_.get(); }
    void initializeRenderer();
    void render();
    
    // Data exchange interface implementation
    virtual bool canReceiveData(const std::string& data_type) const override;
    virtual bool canSendData(const std::string& data_type) const override;
    virtual void* receiveData(void* data, const std::string& data_type) override;
    virtual void* sendData(const std::string& data_type) override;
    
    // Solution identification
    void setName(const std::string& name) { name_ = name; }
    std::string getName() const { return name_; }
    
    // Data exchange with other solutions
    bool canExchangeDataWith(Solution* other_solution, const std::string& data_type);
    void* exchangeDataWith(Solution* other_solution, const std::string& data_type, void* data);
    
protected:
    // Override these methods to define data processing capabilities
    virtual bool canProcessDataType(const std::string& data_type) const;
    virtual void* processIncomingData(void* data, const std::string& data_type);
    virtual void* prepareOutgoingData(const std::string& data_type);
    
private:
    std::string name_;
    ConstructionHistory construction_history_;
    std::vector<std::unique_ptr<Node>> nodes_;
    std::unique_ptr<XTD> xtd_;
    std::unique_ptr<OpenGLRenderer> renderer_;
    std::unique_ptr<TerminalWindow> terminal_;
    
    std::string processTerminalCommand(const std::string& command);
};

#endif // SOLUTION_H

