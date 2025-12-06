#ifndef CONSTRUCTION_HISTORY_H
#define CONSTRUCTION_HISTORY_H

#include <vector>
#include <memory>
#include <string>

class ConstructionStep {
public:
    ConstructionStep(const std::string& operation, void* data = nullptr);
    virtual ~ConstructionStep() = default;
    
    std::string getOperation() const { return operation_; }
    void* getData() const { return data_; }
    void setData(void* data) { data_ = data; }
    
private:
    std::string operation_;
    void* data_;
};

class ConstructionHistory {
public:
    ConstructionHistory();
    virtual ~ConstructionHistory() = default;
    
    void addStep(const std::string& operation, void* data = nullptr);
    void undo();
    void redo();
    void clear();
    
    size_t getStepCount() const { return steps_.size(); }
    size_t getCurrentStepIndex() const { return current_step_index_; }
    ConstructionStep* getStep(size_t index) const;
    
    bool canUndo() const { return current_step_index_ > 0; }
    bool canRedo() const { return current_step_index_ < steps_.size(); }
    
private:
    std::vector<std::unique_ptr<ConstructionStep>> steps_;
    size_t current_step_index_;
};

#endif // CONSTRUCTION_HISTORY_H

