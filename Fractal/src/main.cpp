#define GLEW_STATIC
#include "GLEW/glew.h";
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ImGui/imgui.h"
#include "ImGui/Backends/imgui_impl_glfw.h"
#include "ImGui/Backends/imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> 

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif


struct ShaderSource {
    std::string Vertex;
    std::string Fragment;
};

struct FractalData
{
    float scale = 1.0f;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    int iterations = 500;
};

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static ShaderSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if(line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << "\n";
        }
    }
    return { ss[0].str(), ss[1].str()};
}

static unsigned int ComplileShader(const std::string& source, unsigned int type) 
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int lenght;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &lenght);
        char* message = (char*)alloca(lenght * sizeof(char));
        glGetShaderInfoLog(id, lenght, &lenght, message);
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) 
{
    unsigned int program = glCreateProgram();
    unsigned int vertShader = ComplileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fragShader = ComplileShader(fragmentShader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    FractalData* data = static_cast<FractalData*>(glfwGetWindowUserPointer(window));
    if (data) {
        if (yoffset > 0) data->scale = data->scale * 1.05f;
        else data->scale = data->scale * 0.95f;
        std::cout << "Updated Scroll Value: " << data->scale << std::endl;
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        FractalData* data = static_cast<FractalData*>(glfwGetWindowUserPointer(window));
        data->yOffset += 0.01f / data->scale;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        FractalData* data = static_cast<FractalData*>(glfwGetWindowUserPointer(window));
        data->yOffset -= 0.01f / data->scale;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        FractalData* data = static_cast<FractalData*>(glfwGetWindowUserPointer(window));
        data->xOffset -= 0.01f / data->scale;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        FractalData* data = static_cast<FractalData*>(glfwGetWindowUserPointer(window));
        data->xOffset += 0.01f / data->scale;
    }
}


// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;


    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 900, "I hate Jazz", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glewInit(); //!!!!!!!!

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);



    float positions[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
         -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    ShaderSource source = ParseShader("shaders/Fractal.shader");
    unsigned int shader = CreateShader(source.Vertex, source.Fragment);

    int maxIterationLoc = glGetUniformLocation(shader, "maxIterations");
    int zoomLoc = glGetUniformLocation(shader, "zoom");
    int offsetLoc = glGetUniformLocation(shader, "offset");
    int colorLoc = glGetUniformLocation(shader, "colorYey");
    int tLoc = glGetUniformLocation(shader, "tScale");
    int setLoc = glGetUniformLocation(shader, "set");

    glUseProgram(shader);


    FractalData data;
    float tScale = 1.0f;
    const char* items[] = { "Mandelbrot", "Burning Ship", "Julia"};
    static int item_current = 0;
    glfwSetWindowUserPointer(window, &data);
    glfwSetScrollCallback(window, scroll_callback);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground;

    // Main loop
#ifdef __EMSCRIPTEN__
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        processInput(window);

        {
            ImGui::Begin("Controls", 0, window_flags);

            float label_width = 200.0f + ImGui::GetStyle().ItemSpacing.x;
            ImGui::PushItemWidth(label_width);
            ImVec2 content_size = ImGui::GetWindowSize();
            ImGui::SetWindowSize(ImVec2(0, 0), ImGuiCond_Always);

            ImGui::Text("X - Offset");
            ImGui::DragFloat("##x", &data.xOffset, 0.01f, 0.0f, 0.0f, "%.04f");
            ImGui::Text("Y - Offset");
            ImGui::DragFloat("##y", &data.yOffset, 0.01f, 0.0f, 0.0f, "%.04f");
            ImGui::Text("Scale");
            ImGui::DragFloat("##s", &data.scale, 0.1f, 0.0f, 0.0f, "%.06f");
            ImGui::Text("Iterations");
            ImGui::DragInt("##i", &data.iterations, 1, 10);
            ImGui::Text("TScale");
            ImGui::DragFloat("##t", &tScale, 0.01f, 0.0f, 0.0f, "%.04f");
            ImGui::Combo("##combo", &item_current, items, IM_ARRAYSIZE(items));
            ImGui::SeparatorText("Color");
            static float color[3] = { 0.0f, 0.0f, 1.0f };
            ImGui::ColorEdit3("##c", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoSmallPreview);

            ImGui::PopItemWidth();

            ImGui::End();

            glUniform1i(maxIterationLoc, data.iterations);
            glUniform1f(zoomLoc, data.scale);
            glUniform2f(offsetLoc, data.xOffset, data.yOffset);
            glUniform3f(colorLoc,color[0],color[1],color[2]);
            glUniform1f(tLoc, tScale);
            glUniform1i(setLoc, item_current);
        }


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}