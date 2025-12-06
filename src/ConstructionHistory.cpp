#include "../include/ConstructionHistory.h"

ConstructionStep::ConstructionStep(const std::string& operation, void* data)
    : operation_(operation), data_(data) {
}

ConstructionHistory::ConstructionHistory() : current_step_index_(0) {
}

void ConstructionHistory::addStep(const std::string& operation, void* data) {
    // Remove any steps after current index (redo history)
    steps_.erase(steps_.begin() + current_step_index_, steps_.end());
    
    steps_.push_back(std::make_unique<ConstructionStep>(operation, data));
    current_step_index_ = steps_.size();
}

void ConstructionHistory::undo() {
    if (canUndo()) {
        --current_step_index_;
    }
}

void ConstructionHistory::redo() {
    if (canRedo()) {
        ++current_step_index_;
    }
}

void ConstructionHistory::clear() {
    steps_.clear();
    current_step_index_ = 0;
}

ConstructionStep* ConstructionHistory::getStep(size_t index) const {
    if (index < steps_.size()) {
        return steps_[index].get();
    }
    return nullptr;
}

