#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_strings/gl_strings.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

template <typename... Args>
static void printError(Args... args) noexcept {
  try {
    (std::cerr << ... << args);
  } catch (const std::exception& e) {
    std::fprintf(stderr, "printError failed: %s", e.what());
  }
}

// OpenGL Debug Message Callback
void GLAPIENTRY opengl_message_callback(GLenum source, GLenum type,
                                        GLuint message_id, GLenum severity,
                                        GLsizei length, const GLchar* message,
                                        const void* user_param) {
  printError("(OpenGL Debug Message Callback) id: ", message_id,
             " type: ", gl_strings::type(type),
             " severity: ", gl_strings::severity(severity),
             " source: ", gl_strings::source(source), " message: ", message,
             "\n");
}

auto glfw_error_callback(int error, const char* description) {
  printError("Error ", error, ": ", description, "\n");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

auto ReadFile(const std::string& filename) -> std::string {
  const std::ifstream input(filename, std::ios::binary);
  std::stringstream strBuf;
  strBuf << input.rdbuf();
  if (input.fail() && !input.eof()) {
    printError("Error reading file \"", filename, "\"\n");
    return "";
  }
  return strBuf.str();
};

auto GetShaderFromFile(const std::string& filename, GLenum type) noexcept
    -> unsigned int {
  std::string source_code;
  try {
    source_code = ReadFile(filename);
  } catch (const std::exception& ex) {
    printError("Error reading from file \"", filename.c_str(),
               "\": ", ex.what(), "\n");
    return 0;
  }

  if (source_code.empty()) {
    return 0;
  }

  const auto shader = glCreateShader(type);
  if (shader == 0) {
    return 0;
  }

  const auto* const source_pointer = source_code.data();
  glShaderSource(shader, 1, &(source_pointer), nullptr);

  glCompileShader(shader);
  int compilation_succesful = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_succesful);
  if (compilation_succesful == 0) {
    GLsizei log_length = 0;
    const unsigned int buf_len = 1024;
    GLchar message[buf_len];
    glGetShaderInfoLog(shader, buf_len, &log_length, message);
    printError("Error compiling ", filename, ": ", message, "\n");
  }
  return shader;
}

using Vertex = struct Vertex {
  float x;
  float y;
  float z;
  float tex_u;
  float tex_v;
};

using Texture = struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<Texture> textures;
  unsigned int vao, vbo;
  auto setup() -> void;
  auto draw() -> void;
};

class Model {
  public:
    Model(const std::string& path);
    auto draw(/* Shader &shader */) -> void;
 private:
  std::vector<Mesh> meshes;
  std::string directory;
  std::vector<Texture> textures_loaded;

  auto load_model(const std::string& path);
  auto process_node(aiNode* node, const aiScene* scene);
  auto process_mesh(aiMesh* mesh, const aiScene* scene);

  auto loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                            std::string typeName) -> std::vector<Texture>;
};

auto create_texture_from_file(const std::string& filename) -> unsigned int {
  int width;
  int height;
  int nComponents;
  auto const* image_data =
      stbi_load((filename).c_str(), &width, &height, &nComponents, 4);
  if (image_data == nullptr) {
    printError("Could not load image data\n");
    exit(EXIT_FAILURE);
  }

  // Set up texture
  unsigned int textures[1];
  glCreateTextures(GL_TEXTURE_2D, 1, textures);

  glTextureStorage2D(textures[0], 1, GL_RGBA8, width, height);
  glTextureSubImage2D(textures[0], 0, 0, 0, width, height, GL_RGBA,
                      GL_UNSIGNED_BYTE, image_data);
  stbi_image_free((void*)image_data);

  if (width % 4 != 0) {
    // Set alignment to 1 if pixels cannot be packed in a multiple of 4
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  glTextureParameteri(textures[0], GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(textures[0], GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTextureParameteri(textures[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(textures[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return textures[0];
}

auto load_material_textures(aiMaterial* mat, aiTextureType type,
                            const std::string& type_name)
    -> std::vector<Texture> {
  std::vector<Texture> textures;

  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    Texture texture;
    texture.id = create_texture_from_file("models/" + std::string(str.C_Str()));
    texture.type = type_name;
    texture.path = str.C_Str();
    textures.push_back(texture);
  }

  return textures;
}

auto load_model(const std::string& path)
    -> std::pair<std::vector<Vertex>, std::vector<Texture>> {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

  if (scene == nullptr || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) ||
      scene->mRootNode == nullptr) {
    printError("ASSIMP::ERROR::", importer.GetErrorString(), "\n");
  }

  std::vector<Vertex> vertices;
  std::vector<Texture> textures;

  for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[i];

    for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
      Vertex vertex;
      vertex.x = mesh->mVertices[j].x;
      vertex.y = mesh->mVertices[j].y;
      vertex.z = mesh->mVertices[j].z;

      if (mesh->mTextureCoords[0] != nullptr) {
        vertex.tex_u = mesh->mTextureCoords[0][j].x;
        vertex.tex_v = mesh->mTextureCoords[0][j].y;
      }

      vertices.push_back(vertex);
    }

    if (mesh->mMaterialIndex >= 0) {
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

      std::vector<Texture> diffuse_maps = load_material_textures(
          material, aiTextureType_DIFFUSE, "texture_diffuse");
      textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

      std::vector<Texture> specular_maps = load_material_textures(
          material, aiTextureType_SPECULAR, "texture_specular");
      textures.insert(textures.end(), specular_maps.begin(),
                      specular_maps.end());
    }
  }

  return {vertices, textures};
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    printError("Initialization failed\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetErrorCallback(glfw_error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window =
      glfwCreateWindow(640, 480, EXPERIMENT_NAME, nullptr, nullptr);

  if (window == nullptr) {
    printError("Window creation failed\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  const auto glew_err = glewInit();
  if (GLEW_OK != glew_err) {
    printError("Glew Error: ", glewGetErrorString(glew_err), "\n");
  }

  // Set OpenGL Debug Message Callback
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(opengl_message_callback, nullptr);

  unsigned int vertex_shader =
      GetShaderFromFile("shaders/vertex.glsl", GL_VERTEX_SHADER);
  if (vertex_shader == 0) {
    printError("Error building VERTEX SHADER\n");
    exit(EXIT_FAILURE);
  }
  unsigned int fragment_shader =
      GetShaderFromFile("shaders/fragment.glsl", GL_FRAGMENT_SHADER);
  if (fragment_shader == 0) {
    printError("Error building FRAGMENT SHADER\n");
    exit(EXIT_FAILURE);
  }

  const auto program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  int link_successful = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &link_successful);
  if (link_successful == 0) {
    GLsizei log_length = 0;
    const unsigned int buf_len = 1024;
    GLchar message[buf_len];
    glGetProgramInfoLog(program, buf_len, &log_length, message);
    printError("Error linking program ", program, ": ", message, "\n");
  }
  glUseProgram(program);

  unsigned int vaos[1];
  glCreateVertexArrays(1, vaos);
  unsigned int vbos[1];
  glCreateBuffers(1, vbos);

  // Load Model
  stbi_set_flip_vertically_on_load(1);
  const auto [vertices, textures] = load_model("models/spider.obj");

  glNamedBufferStorage(vbos[0],
                       static_cast<long long>(vertices.size() * sizeof(Vertex)),
                       vertices.data(), 0);

  glBindVertexArray(vaos[0]);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Bind textures
  for (unsigned int i = 0; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  // Projection Matrix
  const auto projection_mat =
      glm::perspective(glm::radians(45.0F), 1.3333F, 0.1F, 10.0F);
  const auto project_mat_uniform = glGetUniformLocation(program, "mProjection");
  glUniformMatrix4fv(project_mat_uniform, 1, GL_FALSE,
                     glm::value_ptr(projection_mat));

  // View Matrix
  const auto view_matrix =
      glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, -2.5F));
  const auto view_matrix_uniform = glGetUniformLocation(program, "mView");
  glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE,
                     glm::value_ptr((view_matrix)));

  // Model Matrix
  const auto set_model_matrix = [](unsigned int program, float y_rotation) {
    const auto scale_matrix =
        glm::scale(glm::mat4(1.0), glm::vec3(0.005F, 0.005F, 0.005F));
    const auto rotate_y_matrix =
        glm::rotate(scale_matrix, y_rotation, glm::vec3(0.0F, 1.0F, 0.0F));
    const auto model_matrix = glm::mat4(rotate_y_matrix);

    const auto model_matrix_uniform = glGetUniformLocation(program, "mModel");
    glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE,
                       glm::value_ptr(model_matrix));
  };

  // Set up buffers
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glEnable(GL_DEPTH_TEST);

  // Set up time calculations
  auto last_time = glfwGetTime();
  const auto y_rotation_speed = 0.5F;
  auto y_rotation = 0.0F;

  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate Time
    auto now_time = glfwGetTime();
    auto delta_time = now_time - last_time;
    last_time = now_time;

    // Calculations with Time
    y_rotation += static_cast<float>(y_rotation_speed * delta_time);
    if (glm::two_pi<float>() < y_rotation) {
      y_rotation = 0;
    }

    // Prepare model matrix
    set_model_matrix(program, y_rotation);

    // Draw and swap buffers
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Delete and free resources
  for (const auto& texture : textures) {
    glDeleteTextures(1, &texture.id);
  }
  glDeleteBuffers(1, vbos);
  glDeleteVertexArrays(1, vaos);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
