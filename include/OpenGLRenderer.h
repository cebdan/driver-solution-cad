#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include <vector>

struct Vertex3D {
    float x, y, z;
    float r, g, b, a;
    float nx, ny, nz;
};

class OpenGLRenderer {
public:
    OpenGLRenderer();
    virtual ~OpenGLRenderer();
    
    bool initialize();
    void shutdown();
    
    void beginRender();
    void endRender();
    
    void setViewport(int width, int height);
    void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
    
    void drawTriangles(const std::vector<Vertex3D>& vertices);
    void drawLines(const std::vector<Vertex3D>& vertices);
    void drawPoints(const std::vector<Vertex3D>& vertices);
    
    void setProjectionMatrix(const float* matrix);
    void setModelViewMatrix(const float* matrix);
    
    void enableDepthTest(bool enable);
    void enableLighting(bool enable);
    
    bool isInitialized() const { return initialized_; }
    
private:
    bool initialized_;
    int viewport_width_;
    int viewport_height_;
    
    void setupOpenGL21();
};

#endif // OPENGL_RENDERER_H

