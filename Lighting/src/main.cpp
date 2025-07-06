#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "program.h"
#include "mesh.h"

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint error_id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
    std::cerr << "OpenGL debug: " << message << "\n";
  }
}

void glfwErrorCallback(const int error, const char* description) {
  std::cerr << "GLFW error: " << description << "\n";
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "Failed to initialize!\n";
    return 1;
  }

  glfwSetErrorCallback(glfwErrorCallback);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "Lighting", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window!\n";
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW!\n";
    return 1;
  }

  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
  }

  const auto program = lighting::Program::Create(
      "shaders/vertex.glsl", "shaders/fragment.glsl");
  if (!program) {
    std::cerr << "Failed to initialize program: " << program.error().message <<
        "\n";

    glfwTerminate();
    return 1;
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE,
                            offsetof(lighting::Vertex, x));
  glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE,
                            offsetof(lighting::Vertex, u));
  glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE,
                            offsetof(lighting::Vertex, nx));

  const auto cube_mesh = []() {
    std::vector<lighting::Vertex> vertices = {
        // Front face
        {.x = -0.5F, .y = -0.5F, .z = 0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = -0.5F, .y = 0.5F, .z = 0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = 0.5F, .y = 0.5F, .z = 0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = 0.5F, .y = -0.5F, .z = 0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},

        // Back face
        {.x = -0.5F, .y = -0.5F, .z = -0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = -0.5F, .y = 0.5F, .z = -0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = 0.5F, .y = 0.5F, .z = -0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = 0.5F, .y = -0.5F, .z = -0.5F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
    };
    const std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        4, 5, 1, 1, 0, 4,
        // Right face
        3, 2, 6, 6, 7, 3,
        // Upper face
        1, 5, 6, 6, 2, 1,
        // Lower face
        4, 0, 3, 3, 7, 4,
    };
    return lighting::Mesh::Create(vertices, indices);
  }();

  if (!cube_mesh) {
    std::cerr << "Failed to initialize mesh: " << cube_mesh.error().message <<
        "\n";
    glfwTerminate();
    return 1;
  }

  const auto plane_mesh = []() {
    const std::vector<lighting::Vertex> vertices = {
        {.x = -1.0F, .y = -1.0F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = -1.0F, .y = 1.0F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = 1.0F, .y = 1.0F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
        {.x = 1.0F, .y = -1.0F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
         .ny = 0.0F, .nz = 0.0F},
    };
    const std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,
    };
    return lighting::Mesh::Create(vertices, indices);
  }();

  using WindowStatus = struct WindowStatus {
    float aspect_ratio;
  };

  WindowStatus window_status = [&window]() {
    int window_width;
    int window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    return WindowStatus{
        .aspect_ratio = static_cast<float>(window_width) / static_cast<float>(
                          window_height)
    };
  }();
  glfwSetWindowUserPointer(window, &window_status);

  const auto window_size_callback = [](GLFWwindow* window, int width,
                                       int height) {
    auto* const window_status_ptr = static_cast<WindowStatus*>(
      glfwGetWindowUserPointer(window));
    if (!window_status_ptr || window_status_ptr->aspect_ratio == 0) {
      return;
    }
    window_status_ptr->aspect_ratio =
        static_cast<float>(width) / static_cast<float>(height);
    glViewport(0, 0, width, height);
  };
  glfwSetWindowSizeCallback(window, window_size_callback);

  const auto view_matrix = []() {
    auto new_view_matrix = glm::mat4(1.0F);
    new_view_matrix = glm::translate(new_view_matrix,
                                     glm::vec3(-0.2F, 0.5F, -4.0F));
    new_view_matrix = glm::rotate(new_view_matrix, glm::radians(15.0F),
                                  glm::vec3(1.0F, 0.0F, 0.0F));
    new_view_matrix = glm::rotate(new_view_matrix, glm::radians(45.0F),
                                  glm::vec3(0.0F, 1.0F, 0.0F));
    return new_view_matrix;
  }();

  if (const auto set_m_view_result = program->SetUniformMatrix(
      "mView", view_matrix); !set_m_view_result) {
    std::cerr << "Failed to set uniform: " << set_m_view_result.error().message
        << "\n";
    glfwTerminate();
    return 1;
  }

  const auto get_delta = []() {
    double current_time = glfwGetTime();
    static double last_time = current_time;
    double delta_time = current_time - last_time;
    last_time = current_time;
    return delta_time;
  };

  const auto rotation_speed = 90.0F; // Speed in degrees per second
  double rotation = rotation_speed * get_delta();

  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  // Rendering loop
  while (glfwWindowShouldClose(window) != GLFW_TRUE) {
    program->Use();
    const auto projection_matrix = glm::perspective(
        glm::radians(45.0F), window_status.aspect_ratio, 0.1F, 100.0F);
    if (const auto set_m_projection_result = program->SetUniformMatrix(
        "mProjection", projection_matrix); !set_m_projection_result) {
      std::cerr << "Failed to set uniform: " << set_m_projection_result.error().
          message
          << "\n";
      glfwTerminate();
      return 1;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    program->Use();
    auto plane_model_matrix = glm::mat4(1.0F);
    plane_model_matrix = glm::translate(plane_model_matrix,
                                        glm::vec3(0.0F, -1.0F, 0.0F));
    plane_model_matrix = glm::rotate(plane_model_matrix,
                                     glm::radians(90.0F),
                                     glm::vec3(1.0F, 0.0F, 0.0F));
    if (const auto set_m_model_result = program->SetUniformMatrix(
        "mModel", plane_model_matrix); !set_m_model_result) {
      std::cerr << "Failed to set uniform: " << set_m_model_result.error().
          message << "\n";
      glfwTerminate();
      return 1;
    }
    if (const auto set_v_color_result = program->SetUniformV3(
        "uColor", glm::vec3(0.2F, 0.2F, 0.8F)); !set_v_color_result) {
      std::cerr << "Failed to set uniform: " << set_v_color_result.error().
          message << "\n";
      glfwTerminate();
      return 1;
    }
    plane_mesh->Draw(*program, vao, 0);

    rotation += rotation_speed * get_delta();
    auto cube_model_matrix = glm::rotate(glm::mat4(1.0F),
                                         glm::radians(
                                             static_cast<float>(
                                               rotation)),
                                         glm::vec3(1.0F, 1.0F, 0.0F));
    if (const auto set_m_model_result = program->SetUniformMatrix(
        "mModel", cube_model_matrix); !set_m_model_result) {
      std::cerr << "Failed to set uniform: " << set_m_model_result.error().
          message << "\n";
      glfwTerminate();
      return 1;
    }

    if (const auto set_v_color_result = program->SetUniformV3(
        "uColor", glm::vec3(0.8F, 0.2F, 0.3F)); !set_v_color_result) {
      std::cerr << "Failed to set uniform: " << set_v_color_result.error().
          message << "\n";
      glfwTerminate();
      return 1;
        }
    cube_mesh->Draw(*program, vao, 0);
    glUseProgram(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}