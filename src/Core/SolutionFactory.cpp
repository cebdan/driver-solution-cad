#include "Core/SolutionFactory.h"
#include <stdexcept>
#include <algorithm>

namespace CADCore {

SolutionFactory& SolutionFactory::instance() {
    static SolutionFactory factory;
    return factory;
}

void SolutionFactory::registerSolution(const std::string& type, CreatorFunc creator) {
    if (creators_.find(type) != creators_.end()) {
        throw std::runtime_error("Solution type already registered: " + type);
    }
    creators_[type] = creator;
}

std::unique_ptr<Solution> SolutionFactory::create(const std::string& type, SolutionID id) {
    auto it = creators_.find(type);
    if (it == creators_.end()) {
        throw std::runtime_error("Solution type not registered: " + type);
    }
    return it->second(id);
}

bool SolutionFactory::isRegistered(const std::string& type) const {
    return creators_.find(type) != creators_.end();
}

std::vector<std::string> SolutionFactory::getRegisteredTypes() const {
    std::vector<std::string> types;
    types.reserve(creators_.size());
    for (const auto& pair : creators_) {
        types.push_back(pair.first);
    }
    return types;
}

} // namespace CADCore

