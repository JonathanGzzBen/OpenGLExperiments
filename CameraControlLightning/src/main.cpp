#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "mesh.h"
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

auto get_cube_mesh()
    -> std::expected<camera_control::Mesh, camera_control::Error> {
  std::vector<camera_control::Vertex> vertices = {

      {-0.5f, -0.5f, -0.5f, 0.0F, 0.0F, 0.0f, 0.0f, -1.0f},
      {0.5f, -0.5f, -0.5f, 1.0F, 0.0F, 0.0f, 0.0f, -1.0f},
      {0.5f, 0.5f, -0.5f, 1.0F, 1.0F, 0.0f, 0.0f, -1.0f},
      {0.5f, 0.5f, -0.5f, 1.0F, 1.0F, 0.0f, 0.0f, -1.0f},
      {-0.5f, 0.5f, -0.5f, 0.0F, 1.0F, 0.0f, 0.0f, -1.0f},
      {-0.5f, -0.5f, -0.5f, 0.0F, 0.0F, 0.0f, 0.0f, -1.0f},

      {-0.5f, -0.5f, 0.5f, 0.0F, 0.0F, 0.0f, 0.0f, 1.0f},
      {0.5f, -0.5f, 0.5f, 1.0F, 0.0F, 0.0f, 0.0f, 1.0f},
      {0.5f, 0.5f, 0.5f, 1.0F, 1.0F, 0.0f, 0.0f, 1.0f},
      {0.5f, 0.5f, 0.5f, 1.0F, 1.0F, 0.0f, 0.0f, 1.0f},
      {-0.5f, 0.5f, 0.5f, 0.0F, 1.0F, 0.0f, 0.0f, 1.0f},
      {-0.5f, -0.5f, 0.5f, 0.0F, 0.0F, 0.0f, 0.0f, 1.0f},

      {-0.5f, 0.5f, 0.5f, 1.0F, 0.0F, -1.0f, 0.0f, 0.0f},
      {-0.5f, 0.5f, -0.5f, 1.0F, 1.0F, -1.0f, 0.0f, 0.0f},
      {-0.5f, -0.5f, -0.5f, 0.0F, 1.0F, -1.0f, 0.0f, 0.0f},
      {-0.5f, -0.5f, -0.5f, 0.0F, 1.0F, -1.0f, 0.0f, 0.0f},
      {-0.5f, -0.5f, 0.5f, 0.0F, 0.0F, -1.0f, 0.0f, 0.0f},
      {-0.5f, 0.5f, 0.5f, 1.0F, 0.0F, -1.0f, 0.0f, 0.0f},

      {0.5f, 0.5f, 0.5f, 1.0F, 0.0F, 1.0f, 0.0f, 0.0f},
      {0.5f, 0.5f, -0.5f, 1.0F, 1.0F, 1.0f, 0.0f, 0.0f},
      {0.5f, -0.5f, -0.5f, 0.0F, 1.0F, 1.0f, 0.0f, 0.0f},
      {0.5f, -0.5f, -0.5f, 0.0F, 1.0F, 1.0f, 0.0f, 0.0f},
      {0.5f, -0.5f, 0.5f, 0.0F, 0.0F, 1.0f, 0.0f, 0.0f},
      {0.5f, 0.5f, 0.5f, 1.0F, 0.0F, 1.0f, 0.0f, 0.0f},

      {-0.5f, -0.5f, -0.5f, 0.0F, 0.0F, 0.0f, -1.0f, 0.0f},
      {0.5f, -0.5f, -0.5f, 0.0F, 0.0F, 0.0f, -1.0f, 0.0f},
      {0.5f, -0.5f, 0.5f, 0.0F, 0.0F, 0.0f, -1.0f, 0.0f},
      {0.5f, -0.5f, 0.5f, 0.0F, 0.0F, 0.0f, -1.0f, 0.0f},
      {-0.5f, -0.5f, 0.5f, 0.0F, 0.0F, 0.0f, -1.0f, 0.0f},
      {-0.5f, -0.5f, -0.5f, 0.0F, 0.0F, 0.0f, -1.0f, 0.0f},

      {-0.5f, 0.5f, -0.5f, 0.0F, 1.0F, 0.0f, 1.0f, 0.0f},
      {0.5f, 0.5f, -0.5f, 1.0F, 1.0F, 0.0f, 1.0f, 0.0f},
      {0.5f, 0.5f, 0.5f, 1.0F, 0.0F, 0.0f, 1.0f, 0.0f},
      {0.5f, 0.5f, 0.5f, 1.0F, 0.0F, 0.0f, 1.0f, 0.0f},
      {-0.5f, 0.5f, 0.5f, 0.0F, 0.0F, 0.0f, 1.0f, 0.0f},
      {-0.5f, 0.5f, -0.5f, 0.0F, 1.0F, 0.0f, 1.0f, 0.0f}

  };
  std::vector<unsigned int> indices;
  for (unsigned int i = 0; i < vertices.size(); i++) {
    indices.emplace_back(i);
  }
  return camera_control::Mesh::Create(vertices, indices);
};

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
      glfwCreateWindow(800, 600, "CameraControl", nullptr, nullptr);
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

  const auto program_lighting = camera_control::Program::Create(
      "shaders/vertex.glsl", "shaders/fragment_lighting_source.glsl");
  if (!program_lighting) {
    std::cerr << "Failed to initialize program: "
              << program_lighting.error().message << "\n";

    glfwTerminate();
    return 1;
  }

  const auto program_objects = camera_control::Program::Create(
      "shaders/vertex.glsl", "shaders/fragment.glsl");
  if (!program_objects) {
    std::cerr << "Failed to initialize program: "
              << program_objects.error().message << "\n";

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
                            offsetof(camera_control::Vertex, x));
  glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE,
                            offsetof(camera_control::Vertex, u));
  glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE,
                            offsetof(camera_control::Vertex, nx));

  const auto cube_mesh = get_cube_mesh();

  if (!cube_mesh) {
    std::cerr << "Failed to initialize mesh: " << cube_mesh.error().message
              << "\n";
    glfwTerminate();
    return 1;
  }

  const auto lighting_source_mesh = get_cube_mesh();
  if (!lighting_source_mesh) {
    std::cerr << "Failed to initialize mesh: "
              << lighting_source_mesh.error().message << "\n";
    glfwTerminate();
    return 1;
  }

  const auto plane_mesh = []() {
    const std::vector<camera_control::Vertex> vertices = {
        {.x = -1.0F,
         .y = 0.0F,
         .z = 1.0F,
         .u = 0.0F,
         .v = 0.0F,
         .nx = 0.0F,
         .ny = 1.0F,
         .nz = 0.0F},
        {.x = -1.0F,
         .y = 0.0F,
         .z = -1.0F,
         .u = 0.0F,
         .v = 0.0F,
         .nx = 0.0F,
         .ny = 1.0F,
         .nz = 0.0F},
        {.x = 1.0F,
         .y = 0.0F,
         .z = -1.0F,
         .u = 0.0F,
         .v = 0.0F,
         .nx = 0.0F,
         .ny = 1.0F,
         .nz = 0.0F},
        {.x = 1.0F,
         .y = 0.0F,
         .z = 1.0F,
         .u = 0.0F,
         .v = 0.0F,
         .nx = 0.0F,
         .ny = 1.0F,
         .nz = 0.0F},
    };
    const std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,
    };
    return camera_control::Mesh::Create(vertices, indices);
  }();

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

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

  constexpr auto light_source_position = glm::vec3(0.3F, 0.9F, 0.8F);

  if (const auto set_light_position_result =
          program_objects->SetUniformV3("lightPosition", light_source_position);
      !set_light_position_result) {
    std::cerr << "Failed to set uniform: "
              << set_light_position_result.error().message << "\n";
    glfwTerminate();
    return 1;
  }

  program_objects->SetUniformV3("material.ambient",
                                glm::vec3(1.0f, 0.5f, 0.31f));
  constexpr int diffuse_texture_unit = 0;
  constexpr int specular_texture_unit = 1;
  program_objects->SetUniform1I("material.diffuse", diffuse_texture_unit);
  program_objects->SetUniform1I("material.specular", specular_texture_unit);
  // program_objects->SetUniformV3("material.specular",
  //                               glm::vec3(0.5f, 0.5f, 0.5f));
  program_objects->SetUniform1F("material.shininess", 32.0F);
  program_objects->SetUniformV3("light.ambient", glm::vec3(0.2F, 0.2F, 0.2F));
  program_objects->SetUniformV3("light.diffuse", glm::vec3(0.5F, 0.5F, 0.5F));
  program_objects->SetUniformV3("light.specular", glm::vec3(1.0F, 1.0F, 1.0F));

  auto texture = get_texture("textures/container2.png");
  if (!texture) {
    std::cerr << "Failed to load texture: " << texture.error() << "\n";
    glfwTerminate();
    return 1;
  }
  auto texture_specular = get_texture("textures/container2_specular.png");
  if (!texture_specular) {
    std::cerr << "Failed to load texture_specular: " << texture.error() << "\n";
    glfwTerminate();
    return 1;
  }

  // Rendering loop
  while (glfwWindowShouldClose(window) != GLFW_TRUE) {
    const auto delta_time = static_cast<float>(get_delta());
    handle_input(delta_time);

    if (const auto set_v_view_pos_result =
            program_objects->SetUniformV3("viewPos", camera_position);
        !set_v_view_pos_result) {
      std::cerr << "Failed to set uniform: "
                << set_v_view_pos_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }

    const auto projection_matrix = glm::perspective(
        glm::radians(45.0F), window_status.aspect_ratio, 0.1F, 100.0F);
    if (const auto set_m_projection_result =
            program_objects->SetUniformMatrix("mProjection", projection_matrix);
        !set_m_projection_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_projection_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }

    const auto view_matrix = [&window, &camera_position, &camera_front,
                              &camera_up]() {
      const auto lookat_matrix = glm::lookAt(
          camera_position, camera_position + camera_front, camera_up);
      return lookat_matrix;
    }();

    if (const auto set_m_view_result =
            program_objects->SetUniformMatrix("mView", view_matrix);
        !set_m_view_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_view_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Lighting source
    program_lighting->Use();
    if (const auto set_m_projection_result = program_lighting->SetUniformMatrix(
            "mProjection", projection_matrix);
        !set_m_projection_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_projection_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }
    if (const auto set_m_view_result =
            program_lighting->SetUniformMatrix("mView", view_matrix);
        !set_m_view_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_view_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }
    auto lighting_source_model_matrix = glm::mat4(1.0F);
    lighting_source_model_matrix =
        glm::translate(lighting_source_model_matrix, light_source_position);
    lighting_source_model_matrix = glm::scale(lighting_source_model_matrix,
                                              glm::vec3(0.25F, 0.25F, 0.25F));

    if (const auto set_m_model_result = program_lighting->SetUniformMatrix(
            "mModel", lighting_source_model_matrix);
        !set_m_model_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_model_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }
    lighting_source_mesh->Draw(*program_lighting, vao, 0);

    // Cube
    auto cube_model_matrix = glm::mat4(1.0F);
    if (const auto set_m_model_result =
            program_objects->SetUniformMatrix("mModel", cube_model_matrix);
        !set_m_model_result) {
      std::cerr << "Failed to set uniform: "
                << set_m_model_result.error().message << "\n";
      glfwTerminate();
      return 1;
    }

    glBindTextureUnit(diffuse_texture_unit, *texture);
    glBindTextureUnit(specular_texture_unit, *texture_specular);
    cube_mesh->Draw(*program_objects, vao, 0);
    glBindTextureUnit(diffuse_texture_unit, 0);
    glBindTextureUnit(specular_texture_unit, 0);

    glUseProgram(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}