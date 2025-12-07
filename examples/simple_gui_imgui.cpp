#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include <cstdlib>
#include <mach-o/dyld.h>
// close_app functionality inline to avoid xtd dependency
namespace {
    void close_application() {
        std::exit(0);
    }
}
#include "../include/imgui_macos_utils.h"

// Solution window type
enum class SolutionType {
    Manager,
    Solution2D,
    Solution3D,
    SolutionDraft,
    SolutionBOM
};

// Solution window structure - each solution runs in its own window
struct SolutionWindow {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    ImGuiContext* imgui_context = nullptr;
    SolutionType type;
    std::string title;
    bool visible = true;
    bool should_close = false;
    
    // Window-specific state
    std::string status_text = "Ready";
    std::string image_path;
    bool image_loaded = false;
    GLuint image_texture = 0;
    int image_width = 0;
    int image_height = 0;
    
    SolutionWindow(SolutionType t, const std::string& ttl) : type(t), title(ttl) {}
    
    ~SolutionWindow() {
        if (imgui_context) {
            ImGui::SetCurrentContext(imgui_context);
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext(imgui_context);
        }
        if (gl_context) {
            SDL_GL_DeleteContext(gl_context);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
};

// Application state
struct AppState {
    std::map<SolutionType, std::unique_ptr<SolutionWindow>> windows;
    bool show_about = false;
    bool show_message_box = false;
    std::string message_box_title;
    std::string message_box_text;
    
    // Global menu bar state (shown in manager window)
    bool show_manager_menu = true;
};

static AppState g_app_state;

// Helper function to open document using system default application
void open_document(const std::string& filename) {
    std::filesystem::path doc_path;
    std::vector<std::filesystem::path> search_paths;
    
    // Current working directory
    search_paths.push_back(std::filesystem::current_path() / filename);
    
    // If in build directory, try parent
    auto cwd = std::filesystem::current_path();
    if (cwd.filename() == "build") {
        search_paths.push_back(cwd.parent_path() / filename);
    }
    
    // Try relative to executable (for .app bundle on macOS)
    char exe_path[1024];
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        std::filesystem::path exe = exe_path;
        auto app_path = exe.parent_path().parent_path().parent_path();
        search_paths.push_back(app_path / filename);
        if (app_path.extension() == ".app") {
            search_paths.push_back(app_path.parent_path() / filename);
        }
    }
    
    // Find first existing file
    bool found = false;
    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            doc_path = path;
            found = true;
            break;
        }
    }
    
    if (found) {
        std::string command = "open \"" + doc_path.string() + "\"";
        std::system(command.c_str());
    }
}

// Load image and create OpenGL texture
bool load_image_texture(SolutionWindow* sol_win, const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    // Make context current
    SDL_GL_MakeCurrent(sol_win->window, sol_win->gl_context);
    
    // Create a simple colored texture as placeholder
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Create a simple 256x256 colored texture
    unsigned char pixels[256 * 256 * 3];
    for (int i = 0; i < 256 * 256; i++) {
        pixels[i * 3 + 0] = 200; // R
        pixels[i * 3 + 1] = 200; // G
        pixels[i * 3 + 2] = 200; // B
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    sol_win->image_texture = texture;
    sol_win->image_width = 256;
    sol_win->image_height = 256;
    sol_win->image_loaded = true;
    
    return true;
}

// Try to load image from common locations
void try_load_image(SolutionWindow* sol_win) {
    std::vector<std::filesystem::path> search_paths;
    auto cwd = std::filesystem::current_path();
    
    search_paths.push_back(cwd / "draft_image.png");
    search_paths.push_back(cwd / "draft_image.jpg");
    search_paths.push_back(cwd / "image.png");
    search_paths.push_back(cwd / "image.jpg");
    
    if (cwd.filename() == "build") {
        search_paths.push_back(cwd.parent_path() / "draft_image.png");
        search_paths.push_back(cwd.parent_path() / "draft_image.jpg");
        search_paths.push_back(cwd.parent_path() / "image.png");
        search_paths.push_back(cwd.parent_path() / "image.jpg");
    }
    
    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            if (load_image_texture(sol_win, path.string())) {
                sol_win->image_path = path.string();
                return;
            }
        }
    }
}

// Show message box (using ImGui modal)
void show_message(const std::string& title, const std::string& text) {
    g_app_state.message_box_title = title;
    g_app_state.message_box_text = text;
    g_app_state.show_message_box = true;
}

// Create a new solution window
SolutionWindow* create_solution_window(SolutionType type, const std::string& title, int width = 800, int height = 600) {
    // GL 3.3 Core + GLSL version 330
    const char* glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow(title.c_str(), 
                                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         width, height, window_flags);
    
    if (window == nullptr) {
        std::cerr << "Error: SDL_CreateWindow failed for " << title << std::endl;
        return nullptr;
    }
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    
    // Setup macOS window style (only for manager window)
    if (type == SolutionType::Manager) {
        setup_macos_window_style(window);
    }
    
    // Setup Dear ImGui context for this window
    IMGUI_CHECKVERSION();
    ImGuiContext* imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Create solution window object
    auto sol_win = std::make_unique<SolutionWindow>(type, title);
    sol_win->window = window;
    sol_win->gl_context = gl_context;
    sol_win->imgui_context = imgui_context;
    
    // Try to load image for manager window
    if (type == SolutionType::Manager) {
        try_load_image(sol_win.get());
    }
    
    SolutionWindow* result = sol_win.get();
    g_app_state.windows[type] = std::move(sol_win);
    
    return result;
}

// Render manager window content
void render_manager_window(SolutionWindow* sol_win) {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Solution manager", nullptr, 
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        
        // Main menu bar
        if (ImGui::BeginMenuBar()) {
            // File menu
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New 2D Document", "Cmd+N")) {
                    show_message("CAD System", "New 2D Document created!\n\nDefault coordinate system: Global CS (0,0,0,0,0,0)");
                    sol_win->status_text = "New 2D Document created";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Open...", "Cmd+O")) {
                    // TODO: Implement file dialog
                    sol_win->status_text = "Open document";
                }
                if (ImGui::MenuItem("Save", "Cmd+S")) {
                    show_message("CAD System", "Document saved!");
                    sol_win->status_text = "Document saved";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Cmd+Q")) {
                    close_application();
                }
                ImGui::EndMenu();
            }
            
            // Manager menu
            if (ImGui::BeginMenu("Manager")) {
                if (ImGui::MenuItem("Open Manager")) {
                    SDL_RaiseWindow(sol_win->window);
                    sol_win->status_text = "Manager opened";
                }
                if (ImGui::MenuItem("2D Solutions")) {
                    // Create or show 2D solution window
                    if (g_app_state.windows.find(SolutionType::Solution2D) == g_app_state.windows.end()) {
                        create_solution_window(SolutionType::Solution2D, "2D Solutions", 1000, 700);
                    } else {
                        SDL_ShowWindow(g_app_state.windows[SolutionType::Solution2D]->window);
                        SDL_RaiseWindow(g_app_state.windows[SolutionType::Solution2D]->window);
                    }
                }
                if (ImGui::MenuItem("3D Solutions")) {
                    if (g_app_state.windows.find(SolutionType::Solution3D) == g_app_state.windows.end()) {
                        create_solution_window(SolutionType::Solution3D, "3D Solutions", 1000, 700);
                    } else {
                        SDL_ShowWindow(g_app_state.windows[SolutionType::Solution3D]->window);
                        SDL_RaiseWindow(g_app_state.windows[SolutionType::Solution3D]->window);
                    }
                }
                if (ImGui::MenuItem("Draft Solutions")) {
                    if (g_app_state.windows.find(SolutionType::SolutionDraft) == g_app_state.windows.end()) {
                        create_solution_window(SolutionType::SolutionDraft, "Draft Solutions", 1000, 700);
                    } else {
                        SDL_ShowWindow(g_app_state.windows[SolutionType::SolutionDraft]->window);
                        SDL_RaiseWindow(g_app_state.windows[SolutionType::SolutionDraft]->window);
                    }
                }
                if (ImGui::MenuItem("BOM Solution")) {
                    if (g_app_state.windows.find(SolutionType::SolutionBOM) == g_app_state.windows.end()) {
                        create_solution_window(SolutionType::SolutionBOM, "BOM Solution", 1000, 700);
                    } else {
                        SDL_ShowWindow(g_app_state.windows[SolutionType::SolutionBOM]->window);
                        SDL_RaiseWindow(g_app_state.windows[SolutionType::SolutionBOM]->window);
                    }
                }
                ImGui::EndMenu();
            }
            
            // Help menu
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("Implementation Notes")) {
                    open_document("IMPLEMENTATION_NOTES.md");
                }
                if (ImGui::MenuItem("Project Goals")) {
                    open_document("PROJECT_GOALS.md");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("About...")) {
                    g_app_state.show_about = true;
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }
        
        // Image display area (left side)
        ImGui::BeginChild("ImageArea", ImVec2(ImGui::GetContentRegionAvail().x - 220, -30), false);
        
        if (sol_win->image_loaded && sol_win->image_texture != 0) {
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();
            ImVec2 image_size(sol_win->image_width, sol_win->image_height);
            
            // Calculate zoom to fit
            float scale = std::min(canvas_size.x / image_size.x, canvas_size.y / image_size.y);
            ImVec2 display_size(image_size.x * scale, image_size.y * scale);
            
            ImGui::SetCursorPosX((canvas_size.x - display_size.x) * 0.5f);
            ImGui::Image((void*)(intptr_t)sol_win->image_texture, display_size);
        } else {
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPosX(canvas_size.x * 0.5f - 100);
            ImGui::SetCursorPosY(canvas_size.y * 0.5f);
            ImGui::Text("No image loaded");
        }
        
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // Buttons area (right side)
        ImGui::BeginChild("ButtonsArea", ImVec2(200, -30), false);
        
        if (ImGui::Button("2d solutions", ImVec2(-1, 40))) {
            if (g_app_state.windows.find(SolutionType::Solution2D) == g_app_state.windows.end()) {
                create_solution_window(SolutionType::Solution2D, "2D Solutions", 1000, 700);
            } else {
                SDL_ShowWindow(g_app_state.windows[SolutionType::Solution2D]->window);
                SDL_RaiseWindow(g_app_state.windows[SolutionType::Solution2D]->window);
            }
        }
        
        ImGui::Spacing();
        if (ImGui::Button("3d solutions", ImVec2(-1, 40))) {
            if (g_app_state.windows.find(SolutionType::Solution3D) == g_app_state.windows.end()) {
                create_solution_window(SolutionType::Solution3D, "3D Solutions", 1000, 700);
            } else {
                SDL_ShowWindow(g_app_state.windows[SolutionType::Solution3D]->window);
                SDL_RaiseWindow(g_app_state.windows[SolutionType::Solution3D]->window);
            }
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Draft solutions", ImVec2(-1, 40))) {
            if (g_app_state.windows.find(SolutionType::SolutionDraft) == g_app_state.windows.end()) {
                create_solution_window(SolutionType::SolutionDraft, "Draft Solutions", 1000, 700);
            } else {
                SDL_ShowWindow(g_app_state.windows[SolutionType::SolutionDraft]->window);
                SDL_RaiseWindow(g_app_state.windows[SolutionType::SolutionDraft]->window);
            }
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Bom solution", ImVec2(-1, 40))) {
            if (g_app_state.windows.find(SolutionType::SolutionBOM) == g_app_state.windows.end()) {
                create_solution_window(SolutionType::SolutionBOM, "BOM Solution", 1000, 700);
            } else {
                SDL_ShowWindow(g_app_state.windows[SolutionType::SolutionBOM]->window);
                SDL_RaiseWindow(g_app_state.windows[SolutionType::SolutionBOM]->window);
            }
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        if (ImGui::Button("Load Image", ImVec2(-1, 40))) {
            // TODO: Implement file dialog
            sol_win->status_text = "Load image (not implemented yet)";
        }
        
        ImGui::EndChild();
        
        // Status bar
        ImGui::Separator();
        ImGui::Text("%s", sol_win->status_text.c_str());
        
        ImGui::End();
    }
}

// Render 2D solution window content
void render_2d_solution_window(SolutionWindow* sol_win) {
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("2D Solutions", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Cmd+N")) {
                    sol_win->status_text = "New 2D document";
                }
                if (ImGui::MenuItem("Open...", "Cmd+O")) {
                    sol_win->status_text = "Open 2D document";
                }
                if (ImGui::MenuItem("Save", "Cmd+S")) {
                    sol_win->status_text = "Save 2D document";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close", "Cmd+W")) {
                    sol_win->should_close = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::Text("2D Solutions Window");
        ImGui::Separator();
        ImGui::Text("Default coordinate system: Global CS (0,0,0,0,0,0)");
        ImGui::Spacing();
        ImGui::Text("This is a dedicated window for 2D solutions.");
        ImGui::Text("Each solution runs independently in its own window.");
        ImGui::Spacing();
        ImGui::Text("Status: %s", sol_win->status_text.c_str());
        
        ImGui::End();
    }
}

// Render 3D solution window content
void render_3d_solution_window(SolutionWindow* sol_win) {
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("3D Solutions", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Cmd+N")) {
                    sol_win->status_text = "New 3D document";
                }
                if (ImGui::MenuItem("Open...", "Cmd+O")) {
                    sol_win->status_text = "Open 3D document";
                }
                if (ImGui::MenuItem("Save", "Cmd+S")) {
                    sol_win->status_text = "Save 3D document";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close", "Cmd+W")) {
                    sol_win->should_close = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::Text("3D Solutions Window");
        ImGui::Separator();
        ImGui::Text("This is a dedicated window for 3D solutions.");
        ImGui::Text("Each solution runs independently in its own window.");
        ImGui::Spacing();
        ImGui::Text("Status: %s", sol_win->status_text.c_str());
        
        ImGui::End();
    }
}

// Render Draft solution window content
void render_draft_solution_window(SolutionWindow* sol_win) {
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Draft Solutions", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Cmd+N")) {
                    sol_win->status_text = "New draft document";
                }
                if (ImGui::MenuItem("Open...", "Cmd+O")) {
                    sol_win->status_text = "Open draft document";
                }
                if (ImGui::MenuItem("Save", "Cmd+S")) {
                    sol_win->status_text = "Save draft document";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close", "Cmd+W")) {
                    sol_win->should_close = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::Text("Draft Solutions Window");
        ImGui::Separator();
        ImGui::Text("This is a dedicated window for draft solutions.");
        ImGui::Text("Each solution runs independently in its own window.");
        ImGui::Spacing();
        ImGui::Text("Status: %s", sol_win->status_text.c_str());
        
        ImGui::End();
    }
}

// Render BOM solution window content
void render_bom_solution_window(SolutionWindow* sol_win) {
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("BOM Solution", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
        
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Cmd+N")) {
                    sol_win->status_text = "New BOM document";
                }
                if (ImGui::MenuItem("Open...", "Cmd+O")) {
                    sol_win->status_text = "Open BOM document";
                }
                if (ImGui::MenuItem("Save", "Cmd+S")) {
                    sol_win->status_text = "Save BOM document";
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close", "Cmd+W")) {
                    sol_win->should_close = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::Text("BOM Solution Window");
        ImGui::Separator();
        ImGui::Text("This is a dedicated window for BOM (Bill of Materials) solutions.");
        ImGui::Text("Each solution runs independently in its own window.");
        ImGui::Spacing();
        ImGui::Text("Status: %s", sol_win->status_text.c_str());
        
        ImGui::End();
    }
}

// Render solution window based on type
void render_solution_window(SolutionWindow* sol_win) {
    ImGui::SetCurrentContext(sol_win->imgui_context);
    
    switch (sol_win->type) {
        case SolutionType::Manager:
            render_manager_window(sol_win);
            break;
        case SolutionType::Solution2D:
            render_2d_solution_window(sol_win);
            break;
        case SolutionType::Solution3D:
            render_3d_solution_window(sol_win);
            break;
        case SolutionType::SolutionDraft:
            render_draft_solution_window(sol_win);
            break;
        case SolutionType::SolutionBOM:
            render_bom_solution_window(sol_win);
            break;
    }
}

int main(int argc, char* argv[]) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    // Create manager window
    SolutionWindow* manager_window = create_solution_window(SolutionType::Manager, "Solution manager", 800, 600);
    if (!manager_window) {
        std::cerr << "Failed to create manager window" << std::endl;
        return -1;
    }
    
    // Main loop
    bool done = false;
    
    while (!done) {
        // Process events for all windows
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Route events to appropriate window
            SDL_Window* event_window = nullptr;
            if (event.type == SDL_WINDOWEVENT) {
                event_window = SDL_GetWindowFromID(event.window.windowID);
            } else if (event.type == SDL_QUIT) {
                // Quit application
                close_application();
                break;
            }
            
            // Process event in all windows (SDL2 handles this automatically)
            for (auto& [type, sol_win] : g_app_state.windows) {
                if (sol_win->window && (!event_window || event_window == sol_win->window)) {
                    ImGui::SetCurrentContext(sol_win->imgui_context);
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    
                    // Handle window close
                    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        if (sol_win->type == SolutionType::Manager) {
                            // Hide manager window instead of closing
                            SDL_HideWindow(sol_win->window);
                            sol_win->visible = false;
                        } else {
                            // Close solution windows
                            sol_win->should_close = true;
                        }
                    }
                    
                    // Handle Cmd+Q (Quit) - only in manager window
                    if (event.type == SDL_KEYDOWN && 
                        (event.key.keysym.mod & KMOD_GUI) && 
                        event.key.keysym.sym == SDLK_q &&
                        sol_win->type == SolutionType::Manager) {
                        close_application();
                    }
                }
            }
        }
        
        // Update and render all windows
        for (auto it = g_app_state.windows.begin(); it != g_app_state.windows.end();) {
            auto& [type, sol_win] = *it;
            
            // Remove closed windows
            if (sol_win->should_close) {
                it = g_app_state.windows.erase(it);
                continue;
            }
            
            // Skip hidden windows
            if (!sol_win->visible && sol_win->type != SolutionType::Manager) {
                ++it;
                continue;
            }
            
            // Make context current
            SDL_GL_MakeCurrent(sol_win->window, sol_win->gl_context);
            ImGui::SetCurrentContext(sol_win->imgui_context);
            
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            
            // Show window if it should be visible
            if (!sol_win->visible && sol_win->type == SolutionType::Manager) {
                SDL_ShowWindow(sol_win->window);
                sol_win->visible = true;
                setup_macos_window_style(sol_win->window);
            }
            
            // Render window content
            render_solution_window(sol_win.get());
            
            // Render message box (only in manager window)
            if (sol_win->type == SolutionType::Manager && g_app_state.show_message_box) {
                ImGui::OpenPopup(g_app_state.message_box_title.c_str());
                if (ImGui::BeginPopupModal(g_app_state.message_box_title.c_str(), nullptr, 
                                          ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextWrapped("%s", g_app_state.message_box_text.c_str());
                    ImGui::Spacing();
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        g_app_state.show_message_box = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            
            // Render about dialog (only in manager window)
            if (sol_win->type == SolutionType::Manager && g_app_state.show_about) {
                ImGui::OpenPopup("About CAD System");
                if (ImGui::BeginPopupModal("About CAD System", &g_app_state.show_about, 
                                          ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("CAD System v1.0");
                    ImGui::Text("Driver Solution CAD");
                    ImGui::Separator();
                    ImGui::Text("Built with ImGui + SDL2");
                    ImGui::Text("Microkernel Architecture - Each solution in separate window");
                    ImGui::Spacing();
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        g_app_state.show_about = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            
            // Rendering
            ImGui::Render();
            ImGuiIO& io = ImGui::GetIO();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(sol_win->window);
            
            ++it;
        }
        
        // Exit if no windows left (except manager)
        if (g_app_state.windows.empty() || 
            (g_app_state.windows.size() == 1 && 
             g_app_state.windows.find(SolutionType::Manager) == g_app_state.windows.end())) {
            done = true;
        }
    }
    
    // Cleanup - windows will be destroyed automatically by unique_ptr
    g_app_state.windows.clear();
    
    SDL_Quit();
    
    return 0;
}
