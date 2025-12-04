#include "UI/Octree.h"
#include <algorithm>
#include <cmath>

namespace CADCore {

Octree::Octree(const BoundingBox& bounds, size_t maxSolutionsPerNode, size_t maxDepth)
    : root_(std::make_unique<Node>(bounds)),
      bounds_(bounds),
      maxSolutionsPerNode_(maxSolutionsPerNode),
      maxDepth_(maxDepth) {
}

void Octree::subdivide(Node* node, int depth) {
    if (depth >= static_cast<int>(maxDepth_) || node->solutions.size() <= maxSolutionsPerNode_) {
        return;
    }
    
    node->isLeaf = false;
    
    // Create 8 children
    for (int i = 0; i < 8; ++i) {
        node->children[i] = std::make_unique<Node>(getChildBounds(node->bounds, i));
    }
    
    // Redistribute solutions to children
    std::vector<SolutionID> solutionsToRedistribute = node->solutions;
    node->solutions.clear();
    
    for (SolutionID id : solutionsToRedistribute) {
        // Find which child this solution belongs to
        // For simplicity, we'll need the bbox - but we don't have it here
        // This is a simplified version - in real implementation, would store bbox with ID
    }
}

void Octree::insert(SolutionID id, const BoundingBox& bbox) {
    if (!root_) {
        return;
    }
    
    insert(root_.get(), id, bbox, 0);
}

void Octree::insert(Node* node, SolutionID id, const BoundingBox& bbox, int depth) {
    // Check if bbox intersects node bounds
    if (bbox.max.x < node->bounds.min.x || bbox.min.x > node->bounds.max.x ||
        bbox.max.y < node->bounds.min.y || bbox.min.y > node->bounds.max.y ||
        bbox.max.z < node->bounds.min.z || bbox.min.z > node->bounds.max.z) {
        return;  // No intersection
    }
    
    if (node->isLeaf) {
        node->solutions.push_back(id);
        
        // Subdivide if needed
        if (node->solutions.size() > maxSolutionsPerNode_ && depth < static_cast<int>(maxDepth_)) {
            subdivide(node, depth);
        }
    } else {
        // Insert into appropriate children
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                insert(node->children[i].get(), id, bbox, depth + 1);
            }
        }
    }
}

void Octree::query(Node* node, const BoundingBox& queryBox, std::vector<SolutionID>& results) const {
    if (!node) {
        return;
    }
    
    // Check if query box intersects node bounds
    if (queryBox.max.x < node->bounds.min.x || queryBox.min.x > node->bounds.max.x ||
        queryBox.max.y < node->bounds.min.y || queryBox.min.y > node->bounds.max.y ||
        queryBox.max.z < node->bounds.min.z || queryBox.min.z > node->bounds.max.z) {
        return;  // No intersection
    }
    
    if (node->isLeaf) {
        // Add all solutions in this leaf
        results.insert(results.end(), node->solutions.begin(), node->solutions.end());
    } else {
        // Query children
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                query(node->children[i].get(), queryBox, results);
            }
        }
    }
}

std::vector<SolutionID> Octree::query(const BoundingBox& queryBox) const {
    std::vector<SolutionID> results;
    if (root_) {
        query(root_.get(), queryBox, results);
    }
    return results;
}

std::vector<SolutionID> Octree::query(const Point3D& point, double radius) const {
    BoundingBox queryBox;
    queryBox.min = Point3D(point.x - radius, point.y - radius, point.z - radius);
    queryBox.max = Point3D(point.x + radius, point.y + radius, point.z + radius);
    return query(queryBox);
}

BoundingBox Octree::getChildBounds(const BoundingBox& parent, int childIndex) const {
    Point3D center = parent.center();
    BoundingBox child;
    
    // Determine which octant
    bool xPos = (childIndex & 1) != 0;
    bool yPos = (childIndex & 2) != 0;
    bool zPos = (childIndex & 4) != 0;
    
    if (xPos) {
        child.min.x = center.x;
        child.max.x = parent.max.x;
    } else {
        child.min.x = parent.min.x;
        child.max.x = center.x;
    }
    
    if (yPos) {
        child.min.y = center.y;
        child.max.y = parent.max.y;
    } else {
        child.min.y = parent.min.y;
        child.max.y = center.y;
    }
    
    if (zPos) {
        child.min.z = center.z;
        child.max.z = parent.max.z;
    } else {
        child.min.z = parent.min.z;
        child.max.z = center.z;
    }
    
    return child;
}

void Octree::clear() {
    // Save bounds before resetting root
    BoundingBox savedBounds = bounds_;
    
    // Reset root to nullptr
    root_.reset();
    
    // Recreate root with saved bounds
    root_ = std::make_unique<Node>(savedBounds);
}

size_t Octree::getNodeCount() const {
    // Simplified - would need to traverse tree
    return 1;  // Placeholder
}

size_t Octree::getSolutionCount() const {
    // Simplified - would need to traverse tree
    return 0;  // Placeholder
}

} // namespace CADCore

