#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <mach-o/dyld.h>
#else
#include <GL/gl.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <deque>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

// ImStudio integration
#ifdef IMSTUDIO_AVAILABLE
#include "ims_gui.h"
#include "../third_party/ImStudio/src/main_window.h"
#endif

// Forward declarations
class Window2D;
class SolutionManagerWindow;
void restartApplication();

// Window types
enum class WindowType {
    SolutionManager,
    Terminal,
    Viewport2D,
    ToolGroupSelector,
    Tools,
    Parameters,
    Variables,
    Layers,
    History,
    Nodes
};

// Base window class - completely independent
class BaseWindow {
public:
    BaseWindow(WindowType type, const std::string& title, int width, int height);
    virtual ~BaseWindow();
    
    bool isOpen() const { return window_ != nullptr && !should_close_; }
    void close() { should_close_ = true; }
    void processEvents();
    void render();
    virtual void update();
    
    SDL_Window* window() const { return window_; }
    WindowType type() const { return type_; }
    const std::string& title() const { return title_; }
    ImGuiContext* imguiContext() const { return imgui_context_; }
    
protected:
    virtual void renderContent() = 0;
    virtual void handleEvent(const SDL_Event& event) {}
    
    WindowType type_;
    std::string title_;
    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    ImGuiContext* imgui_context_ = nullptr;
    bool should_close_ = false;
    int width_, height_;
};

// Forward declaration
class Solution2DWindows;

// UI Editor Window - for editing interface in real-time
class UIEditorWindow : public BaseWindow {
public:
    UIEditorWindow();
    ~UIEditorWindow();
    
protected:
    void renderContent() override;
    
private:
#ifdef IMSTUDIO_AVAILABLE
    std::unique_ptr<ImStudio::GUI> imstudio_gui_;
    bool use_imstudio_ = true;
#else
    bool use_imstudio_ = false;
#endif
};

// Solution Manager Window
class SolutionManagerWindow : public BaseWindow {
public:
    SolutionManagerWindow();
    void launch2DSolution();
    void update() override; // Override to handle pending launches
    void processUIEditorEvents(const SDL_Event& event);
    void handleEvent(const SDL_Event& event) override;
    
protected:
    void renderContent() override;
    
private:
    bool show_terminal_ = true;
    std::string terminal_input_;
    std::deque<std::string> terminal_history_;
    std::deque<std::string> terminal_output_;
    bool pending_launch_2d_ = false;
    bool show_ui_editor_ = false;
    bool pending_ui_editor_ = false;
    bool pending_restart_ = false;
    std::unique_ptr<UIEditorWindow> ui_editor_;
};

// Terminal Window (embedded in Solution Manager)
class TerminalWindow {
public:
    TerminalWindow() : input_buffer_(256, '\0') {
        addOutput("Solution Manager Terminal");
        addOutput("Type 'help' for available commands");
        addOutput("");
    }
    void render(ImVec2 size);
    void addOutput(const std::string& text);
    void processCommand(const std::string& command);
    
private:
    std::vector<char> input_buffer_;
    std::deque<std::string> history_;
    std::deque<std::string> output_;
    int history_index_ = -1;
};

// 2D Solution Windows Manager
class Solution2DWindows {
public:
    static Solution2DWindows& instance();
    void createAllWindows();
    void updateAllWindows();
    void closeAllWindows();
    void processEvent(const SDL_Event& event);
    void resetPositions();
    void resetSizes();
    std::map<WindowType, std::unique_ptr<BaseWindow>>& windows() { return windows_; }
    
private:
    Solution2DWindows() = default;
    std::map<WindowType, std::unique_ptr<BaseWindow>> windows_;
};

// BaseWindow implementation
BaseWindow::BaseWindow(WindowType type, const std::string& title, int width, int height)
    : type_(type), title_(title), width_(width), height_(height) {
    
    // GL 3.3 Core
    const char* glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow(title.c_str(), 
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, window_flags);
    
    if (!window_) {
        std::cerr << "Error: SDL_CreateWindow failed for " << title << std::endl;
        return;
    }
    
    gl_context_ = SDL_GL_CreateContext(window_);
    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(1);
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    imgui_context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context_);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // ledSynthmaster light theme
    ImGuiStyle* style = &ImGui::GetStyle();
    
    // Style settings
    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;
    style->ChildRounding = 4.0f;
    style->PopupRounding = 8.0f;
    style->TabRounding = 4.0f;
    
    // Colors - ledSynthmaster light theme
    style->Colors[ImGuiCol_Text] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.39f, 0.38f, 0.77f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.92f, 0.91f, 0.88f, 0.70f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.58f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.92f, 0.91f, 0.88f, 0.92f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.84f, 0.83f, 0.80f, 0.65f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.99f, 1.00f, 0.40f, 0.78f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.47f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.21f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.90f, 0.91f, 0.00f, 0.78f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 1.00f, 0.00f, 0.80f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.14f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 0.14f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.99f, 1.00f, 0.22f, 0.86f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.25f, 1.00f, 0.00f, 0.76f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 1.00f, 0.00f, 0.86f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 1.00f, 0.00f, 0.78f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
    
    ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

BaseWindow::~BaseWindow() {
    if (imgui_context_) {
        ImGui::SetCurrentContext(imgui_context_);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(imgui_context_);
    }
    if (gl_context_) {
        SDL_GL_DeleteContext(gl_context_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
}

void BaseWindow::processEvents() {
    if (!window_) return;
    
    // Check if window was closed by checking its state
    Uint32 flags = SDL_GetWindowFlags(window_);
    if (!(flags & SDL_WINDOW_SHOWN) && !(flags & SDL_WINDOW_MINIMIZED)) {
        // Window is hidden (closed)
        should_close_ = true;
        return;
    }
    
    // Process ImGui events for this window's context
    // Note: SDL events are processed centrally in main loop
    ImGui::SetCurrentContext(imgui_context_);
    ImGui_ImplSDL2_NewFrame();
}

void BaseWindow::render() {
    if (!window_ || should_close_) return;
    
    // Check if window is minimized
    Uint32 flags = SDL_GetWindowFlags(window_);
    if (flags & SDL_WINDOW_MINIMIZED) {
        return; // Don't render minimized windows
    }
    
    SDL_GL_MakeCurrent(window_, gl_context_);
    ImGui::SetCurrentContext(imgui_context_);
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    renderContent();
    
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window_);
}

void BaseWindow::update() {
    processEvents();
    render();
}

// SolutionManagerWindow implementation
SolutionManagerWindow::SolutionManagerWindow() 
    : BaseWindow(WindowType::SolutionManager, "Solution Manager", 800, 600) {
}

void SolutionManagerWindow::renderContent() {
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Restart Application", "Cmd+R")) {
                pending_restart_ = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Cmd+Q")) {
                should_close_ = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Solutions")) {
            if (ImGui::MenuItem("2D Solution")) {
                pending_launch_2d_ = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("UI Editor")) {
            pending_ui_editor_ = true;
        }
        ImGui::EndMainMenuBar();
    }
    
    // Main content area
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 
                                    ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
    ImGui::Begin("Solution Manager", nullptr, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
                ImGuiWindowFlags_NoBackground);
    
    // Header section
    ImGui::Spacing();
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 300) * 0.5f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Solution Manager");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Solutions grid
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 600) * 0.5f);
    ImGui::BeginGroup();
    
    // 2D Solution button - large and prominent
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.50f, 0.90f, 1.00f));
    if (ImGui::Button("2D Solution", ImVec2(280, 120))) {
        pending_launch_2d_ = true;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    // Placeholder for future solutions
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::Button("3D Solution\n(Coming Soon)", ImVec2(280, 120));
    ImGui::PopStyleVar();
    
    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Terminal section - better styled
    static TerminalWindow terminal;
    ImGui::Separator();
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Terminal", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed)) {
        ImGui::Spacing();
        terminal.render(ImVec2(-1, ImGui::GetContentRegionAvail().y - 20));
    }
    
    ImGui::End();
}

void SolutionManagerWindow::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        // Check for Cmd modifier (GUI key on macOS, Ctrl on other platforms)
        // Check both KMOD_GUI and individual left/right GUI modifiers
        Uint16 mod = event.key.keysym.mod;
        bool has_cmd = (mod & KMOD_GUI) != 0 || 
                       (mod & KMOD_LGUI) != 0 || 
                       (mod & KMOD_RGUI) != 0;
        
        if (has_cmd) {
            if (event.key.keysym.sym == SDLK_q) {
                should_close_ = true;
            } else if (event.key.keysym.sym == SDLK_r) {
                pending_restart_ = true;
            }
        }
    }
}

void SolutionManagerWindow::update() {
    BaseWindow::update();
    
    // Process pending UI Editor toggle after rendering is complete
    if (pending_ui_editor_) {
        pending_ui_editor_ = false;
        show_ui_editor_ = !show_ui_editor_;
        if (show_ui_editor_ && !ui_editor_) {
            ui_editor_ = std::make_unique<UIEditorWindow>();
        } else if (!show_ui_editor_ && ui_editor_) {
            ui_editor_.reset();
        }
    }
    
    // Update UI Editor if open
    if (ui_editor_ && ui_editor_->isOpen()) {
        ui_editor_->update();
    } else if (ui_editor_ && !ui_editor_->isOpen()) {
        ui_editor_.reset();
        show_ui_editor_ = false;
    }
    
    // Process pending restart
    if (pending_restart_) {
        pending_restart_ = false;
        restartApplication();
    }
}

void SolutionManagerWindow::processUIEditorEvents(const SDL_Event& event) {
    if (ui_editor_ && ui_editor_->window()) {
        Uint32 window_id = SDL_GetWindowID(ui_editor_->window());
        
        // Only process events specifically for UI Editor window
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.windowID == window_id) {
                ImGui::SetCurrentContext(ui_editor_->imguiContext());
                ImGui_ImplSDL2_ProcessEvent(&event);
                
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    ui_editor_->close();
                }
            }
        } else {
            // For mouse/keyboard events, only process if UI Editor has focus
            Uint32 flags = SDL_GetWindowFlags(ui_editor_->window());
            if (flags & SDL_WINDOW_INPUT_FOCUS) {
                ImGui::SetCurrentContext(ui_editor_->imguiContext());
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
        }
    }
    
    // Process pending launch after rendering is complete
    if (pending_launch_2d_) {
        pending_launch_2d_ = false;
        launch2DSolution();
    }
}

void SolutionManagerWindow::launch2DSolution() {
    Solution2DWindows::instance().createAllWindows();
    
    // Minimize the manager window
    if (window_) {
        SDL_MinimizeWindow(window_);
    }
}

// TerminalWindow implementation
void TerminalWindow::render(ImVec2 size) {
    // Output area with better styling
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
    if (ImGui::BeginChild("TerminalOutput", ImVec2(size.x, size.y - 50), true, 
                         ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
        for (const auto& line : output_) {
            ImGui::TextUnformatted(line.c_str());
        }
        ImGui::PopStyleColor();
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Input area with prompt
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.18f, 1.00f));
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "> ");
    ImGui::SameLine();
    ImGui::PushItemWidth(size.x - 30);
    if (ImGui::InputText("##TerminalInput", input_buffer_.data(), input_buffer_.size(),
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
        std::string cmd(input_buffer_.data());
        if (!cmd.empty() && cmd[0] != '\0') {
            processCommand(cmd);
            std::fill(input_buffer_.begin(), input_buffer_.end(), '\0');
        }
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
}

void TerminalWindow::addOutput(const std::string& text) {
    output_.push_back(text);
    if (output_.size() > 100) {
        output_.pop_front();
    }
}

void TerminalWindow::processCommand(const std::string& command) {
    addOutput("> " + command);
    if (command == "help" || command == "?") {
        addOutput("Available commands:");
        addOutput("  help, ?     - show this help");
        addOutput("  clear       - clear terminal");
        addOutput("  status      - show system status");
        addOutput("  solutions   - list available solutions");
        addOutput("  restart     - restart the application");
    } else if (command == "clear") {
        output_.clear();
        addOutput("Solution Manager Terminal");
        addOutput("Type 'help' for available commands");
        addOutput("");
    } else if (command == "status") {
        addOutput("System Status: OK");
        addOutput("Core: Ready");
        addOutput("GUI: Active");
    } else if (command == "solutions") {
        addOutput("Available solutions:");
        addOutput("  - 2D Solution");
        addOutput("  - 3D Solution (coming soon)");
    } else if (command == "restart") {
        addOutput("Restarting application...");
        restartApplication();
    } else if (!command.empty()) {
        addOutput("Unknown command: " + command);
        addOutput("Type 'help' for available commands");
    }
}

// 2D Solution Windows - all independent
class Viewport2DWindow : public BaseWindow {
public:
    Viewport2DWindow() : BaseWindow(WindowType::Viewport2D, "2D Viewport (Top View)", 800, 600) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "2D Viewport - Top View");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Viewport area placeholder
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
        ImGui::BeginChild("ViewportCanvas", canvas_size, true, ImGuiWindowFlags_NoScrollbar);
        
        // Grid pattern
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        ImVec2 p1 = ImVec2(p0.x + canvas_size.x, p0.y + canvas_size.y);
        
        // Draw grid
        float grid_size = 20.0f;
        for (float x = p0.x; x < p1.x; x += grid_size) {
            draw_list->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), IM_COL32(40, 40, 50, 255));
        }
        for (float y = p0.y; y < p1.y; y += grid_size) {
            draw_list->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), IM_COL32(40, 40, 50, 255));
        }
        
        // Center text
        ImVec2 text_size = ImGui::CalcTextSize("OpenGL rendering area");
        ImGui::SetCursorPos(ImVec2((canvas_size.x - text_size.x) * 0.5f, 
                                   (canvas_size.y - text_size.y) * 0.5f));
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "OpenGL rendering area");
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
};

class ToolGroupSelectorWindow : public BaseWindow {
public:
    ToolGroupSelectorWindow() : BaseWindow(WindowType::ToolGroupSelector, "Tool Groups", 200, 400) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Tool Groups");
        ImGui::Separator();
        ImGui::Spacing();
        
        const char* groups[] = { "Lines", "Circles", "Primitives", "Modify" };
        for (int i = 0; i < 4; i++) {
            bool is_selected = (selected_group_ == i);
            if (is_selected) {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.80f));
            }
            if (ImGui::Selectable(groups[i], is_selected, 0, ImVec2(0, 30))) {
                selected_group_ = i;
            }
            if (is_selected) {
                ImGui::PopStyleColor();
            }
        }
    }
private:
    int selected_group_ = 0;
};

class ToolsWindow : public BaseWindow {
public:
    ToolsWindow() : BaseWindow(WindowType::Tools, "Tools", 200, 400) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Tools");
        ImGui::Separator();
        ImGui::Spacing();
        
        const char* tools[] = { "Line", "Circle", "Arc", "Rectangle" };
        for (int i = 0; i < 4; i++) {
            if (ImGui::Button(tools[i], ImVec2(-1, 35))) {
                // TODO: Activate tool
            }
            if (i < 3) ImGui::Spacing();
        }
    }
};

class ParametersWindow : public BaseWindow {
public:
    ParametersWindow() : BaseWindow(WindowType::Parameters, "Parameters", 250, 300) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Parameters");
        ImGui::Separator();
        ImGui::Spacing();
        
        static float param1 = 0.0f;
        static float param2 = 0.0f;
        ImGui::Text("Parameter 1");
        ImGui::InputFloat("##param1", &param1);
        ImGui::Spacing();
        ImGui::Text("Parameter 2");
        ImGui::InputFloat("##param2", &param2);
    }
};

class VariablesWindow : public BaseWindow {
public:
    VariablesWindow() : BaseWindow(WindowType::Variables, "Variables", 300, 400) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Variables");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("+ Add Variable", ImVec2(-1, 30))) {
            // TODO: Add variable
        }
        ImGui::PopStyleColor(2);
        
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Variable list will be here");
    }
};

class LayersWindow : public BaseWindow {
public:
    LayersWindow() : BaseWindow(WindowType::Layers, "Layers", 250, 300) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Layers");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("+ Add Layer", ImVec2(-1, 30))) {
            // TODO: Add layer
        }
        ImGui::PopStyleColor(2);
        
        ImGui::Separator();
        ImGui::Spacing();
        
        bool layer1 = true;
        ImGui::Checkbox("Layer 1", &layer1);
        bool layer2 = true;
        ImGui::Checkbox("Layer 2", &layer2);
    }
};

class HistoryWindow : public BaseWindow {
public:
    HistoryWindow() : BaseWindow(WindowType::History, "History", 300, 400) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Construction History");
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.2f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
        if (ImGui::Button("Undo", ImVec2(90, 30))) {
            // TODO: Undo
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("Redo", ImVec2(90, 30))) {
            // TODO: Redo
        }
        ImGui::PopStyleColor(2);
        ImGui::EndGroup();
        
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "History list will be here");
    }
};

class NodesWindow : public BaseWindow {
public:
    NodesWindow() : BaseWindow(WindowType::Nodes, "Nodes View", 400, 500) {}
protected:
    void renderContent() override {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Nodes View");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Visualization of Solution graph");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Node graph area placeholder
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
        ImGui::BeginChild("NodeCanvas", canvas_size, true);
        
        ImVec2 text_size = ImGui::CalcTextSize("Node graph will be displayed here");
        ImGui::SetCursorPos(ImVec2((canvas_size.x - text_size.x) * 0.5f, 
                                   (canvas_size.y - text_size.y) * 0.5f));
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "Node graph will be displayed here");
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
};

// Solution2DWindows implementation
Solution2DWindows& Solution2DWindows::instance() {
    static Solution2DWindows inst;
    return inst;
}

void Solution2DWindows::createAllWindows() {
    // Create all windows - each completely independent
    windows_[WindowType::Viewport2D] = std::make_unique<Viewport2DWindow>();
    windows_[WindowType::ToolGroupSelector] = std::make_unique<ToolGroupSelectorWindow>();
    windows_[WindowType::Tools] = std::make_unique<ToolsWindow>();
    windows_[WindowType::Parameters] = std::make_unique<ParametersWindow>();
    windows_[WindowType::Variables] = std::make_unique<VariablesWindow>();
    windows_[WindowType::Layers] = std::make_unique<LayersWindow>();
    windows_[WindowType::History] = std::make_unique<HistoryWindow>();
    windows_[WindowType::Nodes] = std::make_unique<NodesWindow>();
    
    // Position windows
    auto* viewport = windows_[WindowType::Viewport2D].get();
    if (viewport && viewport->window()) {
        SDL_SetWindowPosition(viewport->window(), 100, 100);
    }
    
    auto* toolGroup = windows_[WindowType::ToolGroupSelector].get();
    if (toolGroup && toolGroup->window()) {
        SDL_SetWindowPosition(toolGroup->window(), 920, 100);
    }
    
    auto* tools = windows_[WindowType::Tools].get();
    if (tools && tools->window()) {
        SDL_SetWindowPosition(tools->window(), 920, 520);
    }
    
    auto* params = windows_[WindowType::Parameters].get();
    if (params && params->window()) {
        SDL_SetWindowPosition(params->window(), 1140, 100);
    }
    
    auto* vars = windows_[WindowType::Variables].get();
    if (vars && vars->window()) {
        SDL_SetWindowPosition(vars->window(), 1140, 420);
    }
    
    auto* layers = windows_[WindowType::Layers].get();
    if (layers && layers->window()) {
        SDL_SetWindowPosition(layers->window(), 1140, 740);
    }
    
    auto* history = windows_[WindowType::History].get();
    if (history && history->window()) {
        SDL_SetWindowPosition(history->window(), 920, 940);
    }
    
    auto* nodes = windows_[WindowType::Nodes].get();
    if (nodes && nodes->window()) {
        SDL_SetWindowPosition(nodes->window(), 1460, 100);
    }
}

void Solution2DWindows::processEvent(const SDL_Event& event) {
    // Distribute event to appropriate window
    for (auto& pair : windows_) {
        BaseWindow* win = pair.second.get();
        if (win && win->window() && 
            event.type == SDL_WINDOWEVENT &&
            event.window.windowID == SDL_GetWindowID(win->window())) {
            // This event is for this window
            ImGui::SetCurrentContext(win->imguiContext());
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                win->close();
            }
            break;
        }
    }
}

void Solution2DWindows::updateAllWindows() {
    for (auto it = windows_.begin(); it != windows_.end();) {
        if (!it->second->isOpen()) {
            // Window is closed - destroy it properly
            it = windows_.erase(it);
        } else {
            it->second->update();
            ++it;
        }
    }
}

void Solution2DWindows::closeAllWindows() {
    windows_.clear();
}

void Solution2DWindows::resetPositions() {
    // Reset to default positions
    auto* viewport = windows_[WindowType::Viewport2D].get();
    if (viewport && viewport->window()) {
        SDL_SetWindowPosition(viewport->window(), 100, 100);
    }
    
    auto* toolGroup = windows_[WindowType::ToolGroupSelector].get();
    if (toolGroup && toolGroup->window()) {
        SDL_SetWindowPosition(toolGroup->window(), 920, 100);
    }
    
    auto* tools = windows_[WindowType::Tools].get();
    if (tools && tools->window()) {
        SDL_SetWindowPosition(tools->window(), 920, 520);
    }
    
    auto* params = windows_[WindowType::Parameters].get();
    if (params && params->window()) {
        SDL_SetWindowPosition(params->window(), 1140, 100);
    }
    
    auto* vars = windows_[WindowType::Variables].get();
    if (vars && vars->window()) {
        SDL_SetWindowPosition(vars->window(), 1140, 420);
    }
    
    auto* layers = windows_[WindowType::Layers].get();
    if (layers && layers->window()) {
        SDL_SetWindowPosition(layers->window(), 1140, 740);
    }
    
    auto* history = windows_[WindowType::History].get();
    if (history && history->window()) {
        SDL_SetWindowPosition(history->window(), 920, 940);
    }
    
    auto* nodes = windows_[WindowType::Nodes].get();
    if (nodes && nodes->window()) {
        SDL_SetWindowPosition(nodes->window(), 1460, 100);
    }
}

void Solution2DWindows::resetSizes() {
    // Reset to default sizes
    std::map<WindowType, std::pair<int, int>> defaultSizes = {
        {WindowType::Viewport2D, {800, 600}},
        {WindowType::ToolGroupSelector, {200, 400}},
        {WindowType::Tools, {200, 400}},
        {WindowType::Parameters, {250, 300}},
        {WindowType::Variables, {300, 400}},
        {WindowType::Layers, {250, 300}},
        {WindowType::History, {300, 400}},
        {WindowType::Nodes, {400, 600}}
    };
    
    for (auto& pair : windows_) {
        if (defaultSizes.count(pair.first)) {
            auto& size = defaultSizes[pair.first];
            if (pair.second && pair.second->window()) {
                SDL_SetWindowSize(pair.second->window(), size.first, size.second);
            }
        }
    }
}

// UIEditorWindow implementation
UIEditorWindow::UIEditorWindow() 
    : BaseWindow(WindowType::SolutionManager, "UI Editor", 1920, 1080) {
#ifdef IMSTUDIO_AVAILABLE
    imstudio_gui_ = std::make_unique<ImStudio::GUI>();
    use_imstudio_ = true;
    // Apply ImStudio style
    MainWindowStyle();
    
    // Set font scale and UI element sizes
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
    
    // Set style scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);
#endif
}

UIEditorWindow::~UIEditorWindow() {
    // ImStudio GUI will be destroyed automatically
}

void UIEditorWindow::renderContent() {
#ifdef IMSTUDIO_AVAILABLE
    if (use_imstudio_ && imstudio_gui_) {
        // Ensure we're using the correct ImGui context for this window
        ImGui::SetCurrentContext(imgui_context_);
        
        // ImStudio needs to control the full window
        // Update display size to match our window before rendering
        ImGuiIO& io = ImGui::GetIO();
        int w, h;
        SDL_GetWindowSize(window_, &w, &h);
        int display_w, display_h;
        SDL_GL_GetDrawableSize(window_, &display_w, &display_h);
        
        // Proper scaling for Retina displays
        float scale_x = display_w > 0 ? (float)display_w / w : 1.0f;
        float scale_y = display_h > 0 ? (float)display_h / h : 1.0f;
        
        // Store original display size to restore later
        ImVec2 original_display_size = io.DisplaySize;
        ImVec2 original_framebuffer_scale = io.DisplayFramebufferScale;
        
        // Set display size only for this window's context
        io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
        io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
        
        // Set font scale
        if (io.FontGlobalScale != 1.0f) {
            io.FontGlobalScale = 1.0f;
        }
        
        // Ensure style scaling is applied
        ImGuiStyle& style = ImGui::GetStyle();
        if (style.WindowPadding.x != 12.0f) {
            style.ScaleAllSizes(1.0f);
        }
        
        // Render ImStudio GUI - it will create its own windows within this context
        // MainWindowGUI will create windows that are scoped to this ImGui context
        MainWindowGUI(*imstudio_gui_);
        
        // Restore original display size (though it shouldn't affect other contexts)
        // io.DisplaySize = original_display_size;
        // io.DisplayFramebufferScale = original_framebuffer_scale;
        
        // Check if ImStudio wants to close
        if (!imstudio_gui_->state) {
            close();
        }
        return;
    }
#endif
    
    // Fallback to basic UI Editor
    ImGui::Begin(title_.c_str(), nullptr, ImGuiWindowFlags_MenuBar);
    
    // Menu bar in UI Editor window
    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("UI Editor")) {
            // Already in UI Editor, do nothing or could toggle visibility
        }
#ifdef IMSTUDIO_AVAILABLE
        if (ImGui::MenuItem("Use ImStudio", nullptr, &use_imstudio_)) {
            if (use_imstudio_ && !imstudio_gui_) {
                imstudio_gui_ = std::make_unique<ImStudio::GUI>();
                MainWindowStyle();
            }
        }
#endif
        ImGui::EndMenuBar();
    }
    
    ImGui::TextColored(ImVec4(0.25f, 1.00f, 0.00f, 1.0f), "UI Editor");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Window size editor
    if (ImGui::CollapsingHeader("Window Sizes", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& windows = Solution2DWindows::instance().windows();
        for (auto& pair : windows) {
            BaseWindow* win = pair.second.get();
            if (!win || !win->window()) continue;
            
            int w, h;
            SDL_GetWindowSize(win->window(), &w, &h);
            
            ImGui::Text("%s", win->title().c_str());
            ImGui::PushID(win);
            if (ImGui::InputInt("Width", &w, 10, 50)) {
                SDL_SetWindowSize(win->window(), w, h);
            }
            if (ImGui::InputInt("Height", &h, 10, 50)) {
                SDL_SetWindowSize(win->window(), w, h);
            }
            ImGui::PopID();
            ImGui::Spacing();
        }
    }
    
    // Window position editor
    if (ImGui::CollapsingHeader("Window Positions", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& windows = Solution2DWindows::instance().windows();
        for (auto& pair : windows) {
            BaseWindow* win = pair.second.get();
            if (!win || !win->window()) continue;
            
            int x, y;
            SDL_GetWindowPosition(win->window(), &x, &y);
            
            ImGui::Text("%s", win->title().c_str());
            ImGui::PushID(win);
            if (ImGui::InputInt("X", &x, 10, 50)) {
                SDL_SetWindowPosition(win->window(), x, y);
            }
            if (ImGui::InputInt("Y", &y, 10, 50)) {
                SDL_SetWindowPosition(win->window(), x, y);
            }
            ImGui::PopID();
            ImGui::Spacing();
        }
    }
    
    // Quick actions
    if (ImGui::CollapsingHeader("Quick Actions")) {
        if (ImGui::Button("Reset All Positions", ImVec2(-1, 0))) {
            Solution2DWindows::instance().resetPositions();
        }
        if (ImGui::Button("Reset All Sizes", ImVec2(-1, 0))) {
            Solution2DWindows::instance().resetSizes();
        }
    }
    
    ImGui::End();
}

// Function to get executable path
std::string getExecutablePath() {
#ifdef __APPLE__
    // On macOS, get the path from the app bundle
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::string(path);
    }
    return "";
#else
    // On other platforms, use /proc/self/exe or similar
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return "";
#endif
}

// Function to restart the application
void restartApplication() {
    std::string exe_path = getExecutablePath();
    
    if (exe_path.empty()) {
        // Fallback: try to use the app bundle path on macOS
        #ifdef __APPLE__
        const char* home = getenv("HOME");
        if (home) {
            // Try to find the app bundle
            exe_path = std::string(home) + "/Documents/driver-solution-cad/build/solution_manager.app/Contents/MacOS/solution_manager";
        }
        #endif
    }
    
    std::cout << "Restarting application from: " << exe_path << std::endl;
    
    if (!exe_path.empty()) {
        // Remove lock file so new instance can start
        const char* home = getenv("HOME");
        if (home) {
            std::string lockfile = std::string(home) + "/.solution_manager.lock";
            unlink(lockfile.c_str());
        }
        
        // Clean up SDL before restarting
        SDL_Quit();
        
        // Execute the application again using execv with proper arguments
        char* argv[] = {const_cast<char*>(exe_path.c_str()), nullptr};
        execv(exe_path.c_str(), argv);
        
        // If execv fails, try execl
        execl(exe_path.c_str(), exe_path.c_str(), (char*)nullptr);
        
        // If both fail, print error
        perror("execv/execl failed");
    }
    
    // If we get here, restart failed - just exit
    std::exit(0);
}

// Function to kill existing instances
void killExistingInstances() {
    const char* home = getenv("HOME");
    if (!home) {
        return; // Can't determine home directory
    }
    
    std::string lockfile = std::string(home) + "/.solution_manager.lock";
    std::ifstream lock(lockfile);
    if (lock.good()) {
        pid_t old_pid;
        lock >> old_pid;
        lock.close();
        
        // Check if process still exists
        if (kill(old_pid, 0) == 0) {
            // Process exists, kill it
            kill(old_pid, SIGTERM);
            // Wait a bit for graceful shutdown
            usleep(500000); // 0.5 seconds
            // Force kill if still running
            if (kill(old_pid, 0) == 0) {
                kill(old_pid, SIGKILL);
            }
        }
    }
    
    // Create new lock file
    std::ofstream new_lock(lockfile);
    new_lock << getpid();
    new_lock.close();
}

// Main
int main(int argc, char* argv[]) {
    // Kill existing instances
    killExistingInstances();
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    SolutionManagerWindow manager;
    
    bool running = true;
    while (running) {
        if (!manager.isOpen()) {
            running = false;
            break;
        }
        
        // Process all SDL events centrally
        SDL_Event event;
        SDL_PumpEvents();
        while (SDL_PollEvent(&event)) {
            // Handle quit event
            if (event.type == SDL_QUIT) {
                manager.close();
                running = false;
                break;
            }
            
            // Handle global keyboard shortcuts (Cmd+Q, Cmd+R) before any other processing
            // This allows them to work regardless of which window has focus
            if (event.type == SDL_KEYDOWN) {
                Uint16 mod = event.key.keysym.mod;
                SDL_Keycode sym = event.key.keysym.sym;
                
                // Check for Cmd modifier (GUI key on macOS)
                bool has_cmd = (mod & KMOD_GUI) != 0 || 
                               (mod & KMOD_LGUI) != 0 || 
                               (mod & KMOD_RGUI) != 0;
                
                // Only process actual key presses (not special key codes)
                // SDLK_r = 114, SDLK_q = 113 are normal ASCII values
                if (has_cmd && (sym == SDLK_q || sym == SDLK_r)) {
                    manager.handleEvent(event);
                    // If quit was triggered, exit the loop
                    if (!manager.isOpen()) {
                        running = false;
                        break;
                    }
                    // Don't pass this event to ImGui
                    continue;
                }
            }
            
            // Process for UI Editor window first (if it exists and has focus)
            manager.processUIEditorEvents(event);
            
            // Process for manager window (only if it's not already processed by UI Editor)
            if (manager.window()) {
                Uint32 manager_window_id = SDL_GetWindowID(manager.window());
                bool is_manager_event = false;
                
                if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.windowID == manager_window_id) {
                        is_manager_event = true;
                    }
                } else {
                    // For mouse/keyboard events, check if manager has focus
                    Uint32 manager_flags = SDL_GetWindowFlags(manager.window());
                    if (manager_flags & SDL_WINDOW_INPUT_FOCUS) {
                        is_manager_event = true;
                    }
                }
                
                if (is_manager_event) {
                    ImGui::SetCurrentContext(manager.imguiContext());
                    
                    // Check if this is a keyboard shortcut before ImGui processes it
                    bool is_shortcut = false;
                    if (event.type == SDL_KEYDOWN) {
                        Uint16 mod = event.key.keysym.mod;
                        bool has_cmd = (mod & KMOD_GUI) != 0 || 
                                       (mod & KMOD_LGUI) != 0 || 
                                       (mod & KMOD_RGUI) != 0;
                        if (has_cmd && (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_r)) {
                            is_shortcut = true;
                            // Handle shortcut before ImGui can intercept it
                            manager.handleEvent(event);
                            // If quit was triggered, exit the loop
                            if (!manager.isOpen()) {
                                running = false;
                                break;
                            }
                        }
                    }
                    
                    // Only process event in ImGui if it's not our shortcut
                    if (!is_shortcut) {
                        ImGui_ImplSDL2_ProcessEvent(&event);
                    }
                    
                    if (event.type == SDL_WINDOWEVENT && 
                        event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        manager.close();
                        running = false;
                        break;
                    }
                }
            }
            
            // Process for 2D Solution windows
            Solution2DWindows::instance().processEvent(event);
            
        }
        
        manager.update();
        Solution2DWindows::instance().updateAllWindows();
        
        SDL_Delay(16); // ~60 FPS
    }
    
    Solution2DWindows::instance().closeAllWindows();
    
    // Remove lock file
    const char* home = getenv("HOME");
    if (home) {
        std::string lockfile = std::string(home) + "/.solution_manager.lock";
        unlink(lockfile.c_str());
    }
    
    SDL_Quit();
    
    return 0;
}

