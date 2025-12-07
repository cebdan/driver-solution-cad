#ifndef SOLUTION_H
#define SOLUTION_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>

// Link types between solutions
enum class LinkType {
    None,       // Independent copy (duplicate)
    Partial,    // Base params sync, additions independent (copy)
    Full        // Everything syncs (propagate)
};

// Sync state
enum class SyncState {
    Active,     // Synchronization enabled
    Excluded    // Temporarily disabled
};

class Solution {
public:
    Solution();
    explicit Solution(const std::string& name);
    virtual ~Solution() = default;

    // Identity
    uint64_t id() const { return id_; }
    const std::string& name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    // Core operation - must be implemented by derived classes
    virtual void solve() = 0;

    // Linking operations
    virtual std::unique_ptr<Solution> duplicate() const = 0;  // Independent copy
    virtual std::unique_ptr<Solution> copy() = 0;             // Partial link
    virtual std::unique_ptr<Solution> propagate() = 0;        // Full link
    
    // Link management
    void exclude();                    // Pause sync
    void restore();                    // Resume sync
    void makeIndependent();            // Break link permanently
    LinkType linkType() const { return link_type_; }
    SyncState syncState() const { return sync_state_; }

    // Tool generation
    virtual std::unique_ptr<Solution> makeSimilar() const;

    // Graph connections (inputs/outputs)
    void addInput(Solution* input);
    void removeInput(Solution* input);
    const std::vector<Solution*>& inputs() const { return inputs_; }
    
    void addOutput(Solution* output);
    void removeOutput(Solution* output);
    const std::vector<Solution*>& outputs() const { return outputs_; }

    // Parent-child (linking hierarchy)
    Solution* parent() const { return parent_; }
    const std::vector<Solution*>& children() const { return children_; }

    // Change propagation
    void markDirty();
    bool isDirty() const { return dirty_; }
    void update();  // Recalculate if dirty

    // Notifications
    using ChangeCallback = std::function<void(Solution*)>;
    void onChanged(ChangeCallback callback);

protected:
    // For derived classes to set up parent-child relationship
    void setParent(Solution* parent);
    void addChild(Solution* child);
    void removeChild(Solution* child);
    
    // Notify all outputs about change
    void notifyOutputs();
    
    // Copy base parameters to target
    virtual void copyBaseTo(Solution* target) const;
    
    // Link type - accessible to derived classes
    LinkType link_type_ = LinkType::None;

private:
    static std::atomic<uint64_t> next_id_;
    
    uint64_t id_;
    std::string name_;
    
    // Graph
    std::vector<Solution*> inputs_;
    std::vector<Solution*> outputs_;
    
    // Linking
    Solution* parent_ = nullptr;
    std::vector<Solution*> children_;
    SyncState sync_state_ = SyncState::Active;
    
    // State
    bool dirty_ = true;
    std::vector<ChangeCallback> callbacks_;
};

#endif // SOLUTION_H
