#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Window.hpp"

GLFWwindow* Window::window;

std::shared_ptr<Graphics> Window::graphics = nullptr;

Window::Window(std::shared_ptr<Graphics> graphics, int width, int height, const char* title) {
    Window::graphics = graphics;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, Window::OnResize);

    // TODO
    // Check for Valid Context
    //if (window == nullptr) {
    //    fprintf(stderr, "Failed to Create OpenGL Context");
    //    return EXIT_FAILURE;
    //}

    glfwMakeContextCurrent(window);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Deat ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150 core"); // TODO: should probably check the available version?
}

void Window::LoopUntilDone() {
    while (!glfwWindowShouldClose(window)) {
        Draw();
    }

    glfwTerminate();
}

void Window::OnResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    graphics->UpdateAspect(glm::ivec2(width, height));
    Draw();
}

void Window::Draw() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glfwPollEvents();

    // Render GUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    graphics->Draw();

    // Add UI stuff here

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Flip Buffers and Draw
    glfwSwapBuffers(window);
}
