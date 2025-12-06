#include "../include/SolutionDocument.h"
#include <algorithm>

SolutionDocument::SolutionDocument()
    : name_("Untitled"), path_(""), modified_(false), author_(""), description_(""), version_("1.0") {
}

void SolutionDocument::addSolution(std::unique_ptr<Solution> solution) {
    if (solution) {
        solutions_.push_back(std::move(solution));
        modified_ = true;
    }
}

Solution* SolutionDocument::getSolution(size_t index) const {
    if (index < solutions_.size()) {
        return solutions_[index].get();
    }
    return nullptr;
}

Solution* SolutionDocument::getSolution(const std::string& name) const {
    for (const auto& solution : solutions_) {
        if (solution->getName() == name) {
            return solution.get();
        }
    }
    return nullptr;
}

void SolutionDocument::removeSolution(size_t index) {
    if (index < solutions_.size()) {
        solutions_.erase(solutions_.begin() + index);
        modified_ = true;
    }
}

void SolutionDocument::removeSolution(const std::string& name) {
    solutions_.erase(
        std::remove_if(solutions_.begin(), solutions_.end(),
            [&name](const std::unique_ptr<Solution>& solution) {
                return solution->getName() == name;
            }),
        solutions_.end()
    );
    modified_ = true;
}

void SolutionDocument::clearSolutions() {
    solutions_.clear();
    modified_ = true;
}

std::vector<Solution*> SolutionDocument::getAllSolutions() const {
    std::vector<Solution*> result;
    for (const auto& solution : solutions_) {
        result.push_back(solution.get());
    }
    return result;
}

