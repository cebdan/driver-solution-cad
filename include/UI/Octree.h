#pragma once

#include "Core/Types.h"
#include "UI/RenderCache.h"
#include <vector>
#include <memory>

namespace CADCore {

/// Octree node for spatial indexing
/// Optimizes spatial queries for 10000+ bodies
class Octree {
private:
    struct Node {
        BoundingBox bounds;
        std::vector<SolutionID> solutions;
        std::unique_ptr<Node> children[8];
        bool isLeaf;
        
        Node(const BoundingBox& bbox) : bounds(bbox), isLeaf(true) {
            for (int i = 0; i < 8; ++i) {
                children[i] = nullptr;
            }
        }
    };
    
    std::unique_ptr<Node> root_;
    BoundingBox bounds_;  // Store original bounds for clear() and reconstruction
    size_t maxSolutionsPerNode_;
    size_t maxDepth_;
    
    void subdivide(Node* node, int depth);
    void insert(Node* node, SolutionID id, const BoundingBox& bbox, int depth);
    void query(Node* node, const BoundingBox& queryBox, std::vector<SolutionID>& results) const;
    BoundingBox getChildBounds(const BoundingBox& parent, int childIndex) const;
    
public:
    Octree(const BoundingBox& bounds, size_t maxSolutionsPerNode = 10, size_t maxDepth = 8);
    
    void insert(SolutionID id, const BoundingBox& bbox);
    void clear();
    std::vector<SolutionID> query(const BoundingBox& queryBox) const;
    std::vector<SolutionID> query(const Point3D& point, double radius) const;
    
    size_t getNodeCount() const;
    size_t getSolutionCount() const;
};

} // namespace CADCore

