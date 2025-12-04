#pragma once

#include "Core/Types.h"
#include "Solutions/ExtrudeSolution.h"
#include "Solutions/RevolveSolution.h"
#include "Solutions/BooleanSolution.h"
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

namespace CADCore {

/// Bounding box for frustum culling
struct BoundingBox {
    Point3D min;
    Point3D max;
    
    BoundingBox() : min(), max() {}
    BoundingBox(const Point3D& minPt, const Point3D& maxPt) : min(minPt), max(maxPt) {}
    
    bool isValid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }
    
    Point3D center() const {
        return Point3D(
            (min.x + max.x) * 0.5,
            (min.y + max.y) * 0.5,
            (min.z + max.z) * 0.5
        );
    }
    
    double radius() const {
        Point3D c = center();
        double dx = max.x - c.x;
        double dy = max.y - c.y;
        double dz = max.z - c.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }
};

/// Geometry cache entry for a Solution
struct GeometryCache {
    SolutionID id;
    std::string type;
    BoundingBox bbox;
    bool isDirty;
    uint32_t vboId;  // OpenGL VBO ID (0 = not created)
    size_t vertexCount;
    size_t indexCount;
    
    GeometryCache() : id(INVALID_SOLUTION), type(), bbox(), 
                      isDirty(true), vboId(0), vertexCount(0), indexCount(0) {}
};

/// Render cache with VBO support and optimizations
/// Handles geometry caching, frustum culling, and LOD
class RenderCache {
private:
    std::map<SolutionID, GeometryCache> cache_;
    std::vector<SolutionID> visibleSolutions_;
    bool useVBO_;
    bool useFrustumCulling_;
    bool useLOD_;
    
    // Frustum planes (for culling)
    struct FrustumPlane {
        float a, b, c, d;  // Plane equation: ax + by + cz + d = 0
    };
    std::vector<FrustumPlane> frustumPlanes_;
    
    // LOD distances
    float lodDistance1_;  // Switch to medium detail
    float lodDistance2_;  // Switch to low detail
    
public:
    RenderCache();
    ~RenderCache();
    
    // Cache management
    void markDirty(SolutionID id);
    void updateCache(SolutionID id, const std::string& type, const BoundingBox& bbox);
    GeometryCache* getCache(SolutionID id);
    bool isCached(SolutionID id) const;
    
    // VBO management
    void createVBO(SolutionID id, const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
    void deleteVBO(SolutionID id);
    void bindVBO(SolutionID id);
    void unbindVBO();
    
    // Frustum culling
    void updateFrustum(float fov, float aspect, float nearPlane, float farPlane,
                       const float* viewMatrix, const float* projMatrix);
    bool isVisible(const BoundingBox& bbox) const;
    std::vector<SolutionID> getVisibleSolutions(const std::vector<SolutionID>& allIDs);
    
    // LOD
    int getLODLevel(const BoundingBox& bbox, const Point3D& cameraPos) const;
    float getLODDistance(const BoundingBox& bbox, const Point3D& cameraPos) const;
    
    // Options
    void setUseVBO(bool use) { useVBO_ = use; }
    void setUseFrustumCulling(bool use) { useFrustumCulling_ = use; }
    void setUseLOD(bool use) { useLOD_ = use; }
    bool getUseVBO() const { return useVBO_; }
    bool getUseFrustumCulling() const { return useFrustumCulling_; }
    bool getUseLOD() const { return useLOD_; }
    
    // Cleanup
    void clear();
    void cleanup();
};

} // namespace CADCore

