#include "Core/Solution.h"
#include <algorithm>
#include <stdexcept>

namespace CADCore {

Solution::Solution(SolutionID id, const std::string& type)
    : id_(id), type_(type), dirty_(true) {
}

void Solution::setDriver(const std::string& name, std::any value) {
    drivers_[name] = value;
    dirty_ = true;
}

std::any Solution::getDriver(const std::string& name) const {
    auto it = drivers_.find(name);
    if (it == drivers_.end()) {
        throw std::runtime_error("Driver not found: " + name);
    }
    return it->second;
}

bool Solution::hasDriver(const std::string& name) const {
    return drivers_.find(name) != drivers_.end();
}

void Solution::setOutput(const std::string& name, std::any value) {
    outputs_[name] = value;
}

std::any Solution::getOutput(const std::string& name) const {
    auto it = outputs_.find(name);
    if (it == outputs_.end()) {
        throw std::runtime_error("Output not found: " + name);
    }
    return it->second;
}

bool Solution::hasOutput(const std::string& name) const {
    return outputs_.find(name) != outputs_.end();
}

void Solution::execute(Kernel* kernel) {
    if (!dirty_) {
        return;
    }
    
    validateDrivers();
    compute(kernel);
    dirty_ = false;
}

void Solution::validateDrivers() const {
    auto required = getRequiredDrivers();
    for (const auto& driverName : required) {
        if (!hasDriver(driverName)) {
            throw std::runtime_error("Required driver missing: " + driverName);
        }
    }
}

} // namespace CADCore

