#include "Solution.h"
#include <algorithm>

std::atomic<uint64_t> Solution::next_id_{1};

Solution::Solution() 
    : id_(next_id_++), name_("Solution") {}

Solution::Solution(const std::string& name) 
    : id_(next_id_++), name_(name) {}

void Solution::exclude() {
    sync_state_ = SyncState::Excluded;
}

void Solution::restore() {
    sync_state_ = SyncState::Active;
    if (parent_ && link_type_ != LinkType::None) {
        markDirty();
    }
}

void Solution::makeIndependent() {
    if (parent_) {
        parent_->removeChild(this);
        parent_ = nullptr;
    }
    link_type_ = LinkType::None;
}

std::unique_ptr<Solution> Solution::makeSimilar() const {
    // Default implementation - override in derived classes
    // Returns a "recipe" that can be applied to other inputs
    return nullptr;
}

void Solution::addInput(Solution* input) {
    if (input && std::find(inputs_.begin(), inputs_.end(), input) == inputs_.end()) {
        inputs_.push_back(input);
        input->addOutput(this);
    }
}

void Solution::removeInput(Solution* input) {
    auto it = std::find(inputs_.begin(), inputs_.end(), input);
    if (it != inputs_.end()) {
        inputs_.erase(it);
        input->removeOutput(this);
    }
}

void Solution::addOutput(Solution* output) {
    if (output && std::find(outputs_.begin(), outputs_.end(), output) == outputs_.end()) {
        outputs_.push_back(output);
    }
}

void Solution::removeOutput(Solution* output) {
    auto it = std::find(outputs_.begin(), outputs_.end(), output);
    if (it != outputs_.end()) {
        outputs_.erase(it);
    }
}

void Solution::setParent(Solution* parent) {
    if (parent_ && parent_ != parent) {
        parent_->removeChild(this);
    }
    parent_ = parent;
    if (parent) {
        parent->addChild(this);
    }
}

void Solution::addChild(Solution* child) {
    if (child && std::find(children_.begin(), children_.end(), child) == children_.end()) {
        children_.push_back(child);
    }
}

void Solution::removeChild(Solution* child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
    }
}

void Solution::markDirty() {
    if (!dirty_) {
        dirty_ = true;
        notifyOutputs();
    }
}

void Solution::update() {
    if (dirty_) {
        // First update all inputs
        for (auto* input : inputs_) {
            input->update();
        }
        
        // Sync from parent if linked
        if (parent_ && sync_state_ == SyncState::Active) {
            if (link_type_ == LinkType::Full) {
                parent_->copyBaseTo(this);
            } else if (link_type_ == LinkType::Partial) {
                parent_->copyBaseTo(this);
            }
        }
        
        // Recalculate
        solve();
        dirty_ = false;
        
        // Notify callbacks
        for (auto& cb : callbacks_) {
            cb(this);
        }
    }
}

void Solution::notifyOutputs() {
    for (auto* output : outputs_) {
        output->markDirty();
    }
    // Also notify children
    for (auto* child : children_) {
        if (child->syncState() == SyncState::Active) {
            child->markDirty();
        }
    }
}

void Solution::onChanged(ChangeCallback callback) {
    callbacks_.push_back(std::move(callback));
}

void Solution::copyBaseTo(Solution* target) const {
    // Base implementation - override in derived classes
    target->name_ = name_;
}

