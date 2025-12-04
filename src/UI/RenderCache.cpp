#include "UI/RenderCache.h"
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <cmath>
#include <algorithm>

namespace CADCore {

RenderCache::RenderCache()
    : useVBO_(true), useFrustumCulling_(true), useLOD_(true),
      lodDistance1_(10.0f), lodDistance2_(50.0f) {
    frustumPlanes_.resize(6);
}

RenderCache::~RenderCache() {
    cleanup();
}

void RenderCache::markDirty(SolutionID id) {
    auto it = cache_.find(id);
    if (it != cache_.end()) {
        it->second.isDirty = true;
        if (it->second.vboId != 0) {
            deleteVBO(id);
        }
    }
}

void RenderCache::updateCache(SolutionID id, const std::string& type, const BoundingBox& bbox) {
    auto it = cache_.find(id);
    if (it == cache_.end()) {
        cache_[id] = GeometryCache();
        it = cache_.find(id);
    }
    
    it->second.id = id;
    it->second.type = type;
    it->second.bbox = bbox;
    it->second.isDirty = false;
}

GeometryCache* RenderCache::getCache(SolutionID id) {
    auto it = cache_.find(id);
    if (it != cache_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool RenderCache::isCached(SolutionID id) const {
    return cache_.find(id) != cache_.end();
}

void RenderCache::createVBO(SolutionID id, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
    auto it = cache_.find(id);
    if (it == cache_.end()) {
        return;
    }
    
    GeometryCache& cache = it->second;
    
    // Delete existing VBO if any
    if (cache.vboId != 0) {
        deleteVBO(id);
    }
    
    if (!useVBO_ || vertices.empty()) {
        return;
    }
    
    // Generate VBO IDs (simplified - in real implementation would use proper OpenGL 3.3+)
    // For OpenGL 2.1, we'll use display lists or just store data
    // Note: OpenGL 2.1 doesn't have VBO in the same way, but we can prepare for upgrade
    cache.vertexCount = vertices.size() / 3;  // Assuming 3 floats per vertex
    cache.indexCount = indices.size();
    
    // For now, just mark as ready (VBO creation would require OpenGL 3.3+)
    // In OpenGL 2.1, we use immediate mode, but cache the data
    cache.vboId = 1;  // Placeholder - indicates data is cached
}

void RenderCache::deleteVBO(SolutionID id) {
    auto it = cache_.find(id);
    if (it != cache_.end()) {
        GeometryCache& cache = it->second;
        if (cache.vboId != 0) {
            // In OpenGL 2.1, no actual VBO to delete
            // But we clear the cache
            cache.vboId = 0;
            cache.vertexCount = 0;
            cache.indexCount = 0;
        }
    }
}

void RenderCache::bindVBO(SolutionID id) {
    // For OpenGL 2.1, this is a no-op
    // In OpenGL 3.3+, would bind the actual VBO
}

void RenderCache::unbindVBO() {
    // For OpenGL 2.1, this is a no-op
}

void RenderCache::updateFrustum(float fov, float aspect, float nearPlane, float farPlane,
                                const float* viewMatrix, const float* projMatrix) {
    if (!useFrustumCulling_) {
        return;
    }
    
    // Extract frustum planes from view-projection matrix
    // Combined matrix = projMatrix * viewMatrix
    float combined[16];
    
    // Multiply matrices (simplified - assumes column-major)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            combined[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                combined[i * 4 + j] += projMatrix[i * 4 + k] * viewMatrix[k * 4 + j];
            }
        }
    }
    
    // Extract planes (normalize)
    // Left plane
    frustumPlanes_[0].a = combined[3] + combined[0];
    frustumPlanes_[0].b = combined[7] + combined[4];
    frustumPlanes_[0].c = combined[11] + combined[8];
    frustumPlanes_[0].d = combined[15] + combined[12];
    
    // Right plane
    frustumPlanes_[1].a = combined[3] - combined[0];
    frustumPlanes_[1].b = combined[7] - combined[4];
    frustumPlanes_[1].c = combined[11] - combined[8];
    frustumPlanes_[1].d = combined[15] - combined[12];
    
    // Bottom plane
    frustumPlanes_[2].a = combined[3] + combined[1];
    frustumPlanes_[2].b = combined[7] + combined[5];
    frustumPlanes_[2].c = combined[11] + combined[9];
    frustumPlanes_[2].d = combined[15] + combined[13];
    
    // Top plane
    frustumPlanes_[3].a = combined[3] - combined[1];
    frustumPlanes_[3].b = combined[7] - combined[5];
    frustumPlanes_[3].c = combined[11] - combined[9];
    frustumPlanes_[3].d = combined[15] - combined[13];
    
    // Near plane
    frustumPlanes_[4].a = combined[3] + combined[2];
    frustumPlanes_[4].b = combined[7] + combined[6];
    frustumPlanes_[4].c = combined[11] + combined[10];
    frustumPlanes_[4].d = combined[15] + combined[14];
    
    // Far plane
    frustumPlanes_[5].a = combined[3] - combined[2];
    frustumPlanes_[5].b = combined[7] - combined[6];
    frustumPlanes_[5].c = combined[11] - combined[10];
    frustumPlanes_[5].d = combined[15] - combined[14];
    
    // Normalize all planes
    for (int i = 0; i < 6; ++i) {
        float len = sqrtf(frustumPlanes_[i].a * frustumPlanes_[i].a +
                          frustumPlanes_[i].b * frustumPlanes_[i].b +
                          frustumPlanes_[i].c * frustumPlanes_[i].c);
        if (len > 0.0001f) {
            frustumPlanes_[i].a /= len;
            frustumPlanes_[i].b /= len;
            frustumPlanes_[i].c /= len;
            frustumPlanes_[i].d /= len;
        }
    }
}

bool RenderCache::isVisible(const BoundingBox& bbox) const {
    if (!useFrustumCulling_ || !bbox.isValid()) {
        return true;
    }
    
    // Test bounding box against all frustum planes
    for (const auto& plane : frustumPlanes_) {
        // Find the "positive" vertex (furthest in direction of plane normal)
        Point3D positiveVertex;
        if (plane.a >= 0) positiveVertex.x = bbox.max.x;
        else positiveVertex.x = bbox.min.x;
        if (plane.b >= 0) positiveVertex.y = bbox.max.y;
        else positiveVertex.y = bbox.min.y;
        if (plane.c >= 0) positiveVertex.z = bbox.max.z;
        else positiveVertex.z = bbox.min.z;
        
        // If positive vertex is behind plane, box is outside frustum
        float distance = plane.a * positiveVertex.x + 
                         plane.b * positiveVertex.y + 
                         plane.c * positiveVertex.z + 
                         plane.d;
        
        if (distance < 0.0f) {
            return false;  // Box is outside frustum
        }
    }
    
    return true;  // Box is inside or intersects frustum
}

std::vector<SolutionID> RenderCache::getVisibleSolutions(const std::vector<SolutionID>& allIDs) {
    if (!useFrustumCulling_) {
        return allIDs;
    }
    
    visibleSolutions_.clear();
    visibleSolutions_.reserve(allIDs.size());
    
    for (SolutionID id : allIDs) {
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            if (isVisible(it->second.bbox)) {
                visibleSolutions_.push_back(id);
            }
        } else {
            // If not cached, assume visible (will be cached on first render)
            visibleSolutions_.push_back(id);
        }
    }
    
    return visibleSolutions_;
}

int RenderCache::getLODLevel(const BoundingBox& bbox, const Point3D& cameraPos) const {
    if (!useLOD_ || !bbox.isValid()) {
        return 0;  // Full detail
    }
    
    float distance = getLODDistance(bbox, cameraPos);
    
    if (distance > lodDistance2_) {
        return 2;  // Low detail
    } else if (distance > lodDistance1_) {
        return 1;  // Medium detail
    }
    
    return 0;  // Full detail
}

float RenderCache::getLODDistance(const BoundingBox& bbox, const Point3D& cameraPos) const {
    Point3D center = bbox.center();
    double dx = center.x - cameraPos.x;
    double dy = center.y - cameraPos.y;
    double dz = center.z - cameraPos.z;
    return static_cast<float>(sqrt(dx*dx + dy*dy + dz*dz));
}

void RenderCache::clear() {
    cleanup();
    cache_.clear();
    visibleSolutions_.clear();
}

void RenderCache::cleanup() {
    // Delete all VBOs
    for (auto& pair : cache_) {
        if (pair.second.vboId != 0) {
            deleteVBO(pair.first);
        }
    }
}

} // namespace CADCore

