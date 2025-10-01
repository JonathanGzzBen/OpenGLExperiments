#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "mesh.h"
#include "model.h"
#include "program.h"

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

auto get_image_data(const std::string& filename, int n_components)
    -> std::expected<
        std::tuple<std::unique_ptr<unsigned char, decltype(&stbi_image_free)>,
                   int, int>,
        std::string> {
  int x, y, n;
  unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, n_components);
  if (data == nullptr) {
    return std::unexpected(std::format("Could not load texture '{}': {}",
                                       filename, stbi_failure_reason()));
  }

  std::unique_ptr<unsigned char, decltype(&stbi_image_free)> tex_data(
      data, &stbi_image_free);
  return {{std::move(tex_data), x, y}};
};

auto get_texture(const std::string& filename)
    -> std::expected<unsigned int, std::string> {
  const auto image_data = get_image_data(filename, 3);
  if (!image_data) {
    return std::unexpected(std::format("Could not load texture '{}': {}",
                                       filename, image_data.error()));
  }
  const auto& [data, width, height] = *image_data;

  unsigned int texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glTextureStorage2D(texture, 1, GL_RGB8, width, height);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
                      data.get());
  return {texture};
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

  GLFWwindow* window =
      glfwCreateWindow(800, 600, "Model Loading", nullptr, nullptr);
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

  const auto program = model_loading::Program::Create("shaders/vertex.glsl",
                                                      "shaders/fragment.glsl");
  if (!program) {
    std::cerr << "Failed to initialize program: " << program.error().message
              << "\n";

    glfwTerminate();
    return 1;
  }

  using WindowStatus = struct WindowStatus {
    float aspect_ratio;
  };

  WindowStatus window_status = [&window]() {
    int window_width;
    int window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    return WindowStatus{
        .aspect_ratio = static_cast<float>(window_width) /
                        static_cast<float>(window_height),
    };
  }();
  glfwSetWindowUserPointer(window, &window_status);

  const auto window_size_callback = [](GLFWwindow* window, int width,
                                       int height) {
    auto* const window_status_ptr =
        static_cast<WindowStatus*>(glfwGetWindowUserPointer(window));
    if (!window_status_ptr || window_status_ptr->aspect_ratio == 0) {
      return;
    }
    window_status_ptr->aspect_ratio =
        static_cast<float>(width) / static_cast<float>(height);
    glViewport(0, 0, width, height);
  };
  glfwSetWindowSizeCallback(window, window_size_callback);

  auto camera_position = glm::vec3(0.0F, 0.2F, 3.0F);
  auto camera_front = glm::vec3(0.0F, 0.0F, -1.0F);
  constexpr auto camera_up = glm::vec3(0.0F, 1.0F, 0.0F);
  auto pitch = 0.0F;
  auto yaw = -90.0F;
  const auto handle_input = [&window, &camera_position, &camera_front,
                             &camera_up, &pitch, &yaw](const float delta_time) {
    auto movement_direction = glm::vec3(0.0F, 0.0F, 0.0F);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      movement_direction += camera_front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      movement_direction -= camera_front;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
      movement_direction -= camera_up;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
      movement_direction += camera_up;
    }
    const auto right = glm::normalize(glm::cross(camera_front, camera_up));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      movement_direction -= right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      movement_direction += right;
    }
    if (movement_direction != glm::vec3(0.0F, 0.0F, 0.0F)) {
      static constexpr float movement_speed = 1.0F;
      camera_position +=
          delta_time * movement_speed * glm::normalize(movement_direction);
    }

    // Camera rotation
    static constexpr auto rotation_speed = 40.0F;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      pitch -= rotation_speed * delta_time;
      if (pitch <= -90.0F) {
        pitch = -89.9F;
      }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      pitch += rotation_speed * delta_time;
      if (90 <= pitch) {
        pitch = 89.9F;
      }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      yaw -= rotation_speed * delta_time;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      yaw += rotation_speed * delta_time;
    }
    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction.y = glm::sin(glm::radians(pitch));
    direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    camera_front = glm::normalize(direction);
  };

  const auto get_delta = []() -> double {
    double current_time = glfwGetTime();
    static double last_time = current_time;
    double delta_time = current_time - last_time;
    last_time = current_time;
    return delta_time;
  };

  // model_loading::Model backpack_model("models/backpack/backpack.obj");
  model_loading::Model backpack_model("models/bunny/bunny.obj");
  // model_loading::Model backpack_model("models/holodeck/holodeck.obj");
  // model_loading::Model backpack_model("models/dragon/dragon.obj");

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

  // Rendering loop
  while (glfwWindowShouldClose(window) != GLFW_TRUE) {
    const auto delta_time = static_cast<float>(get_delta());
    handle_input(delta_time);

    const auto projection_matrix = glm::perspective(
        glm::radians(45.0F), window_status.aspect_ratio, 0.1F, 100.0F);
    if (const auto set_m_projection_result =
            program->SetUniformMatrix("mProjection", projection_matrix);
        !set_m_projection_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_projection_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }

    const auto view_matrix =
        glm::lookAt(camera_position, camera_position + camera_front, camera_up);
    if (const auto set_m_view_result =
            program->SetUniformMatrix("mView", view_matrix);
        !set_m_view_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_view_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }

    auto model_matrix = glm::mat4(1.0F);
    // model_matrix = glm::scale(model_matrix, glm::vec3(0.001F, 0.001F, 0.001F));
    if (const auto res = program->SetUniformMatrix("mModel", model_matrix);
        !res) {
      std::cerr << "Failed to set uniform: " << res.error().message << "\n";
      glfwTerminate();
      return 1;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program->Use();
    backpack_model.Draw(*program);

    glUseProgram(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}