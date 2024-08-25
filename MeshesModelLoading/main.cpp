#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "assimp/material.h"

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

class Shader {
 public:
  unsigned int id;
  Shader(const char* vertex_path, const char* fragment_path);
  auto use() const -> void;
  auto set_int(const std::string& name, int value) const -> void;
  auto set_mat4(const std::string& name, const glm::mat4& mat) const -> void;

  static auto check_compile_errors(unsigned int shader, const std::string& type)
      -> void;
};

Shader::Shader(const char* vertex_path, const char* fragment_path) {
  std::string vertex_code;
  std::string fragment_code;

  std::ifstream v_shader_file;
  std::ifstream f_shader_file;

  v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    v_shader_file.open(vertex_path);
    f_shader_file.open(fragment_path);

    std::stringstream v_shader_stream;
    std::stringstream f_shader_stream;

    v_shader_stream << v_shader_file.rdbuf();
    f_shader_stream << f_shader_file.rdbuf();

    v_shader_file.close();
    f_shader_file.close();

    vertex_code = v_shader_stream.str();
    fragment_code = f_shader_stream.str();
  } catch (std::ifstream::failure& e) {
    std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
  }

  const char* v_shader_code = vertex_code.c_str();
  const char* f_shader_code = fragment_code.c_str();

  unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &v_shader_code, nullptr);
  glCompileShader(vertex);
  check_compile_errors(vertex, "VERTEX");

  unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &f_shader_code, nullptr);
  glCompileShader(fragment);
  check_compile_errors(fragment, "FRAGMENT");

  id = glCreateProgram();
  glAttachShader(id, vertex);
  glAttachShader(id, fragment);

  glLinkProgram(id);
  check_compile_errors(id, "PROGRAM");

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

auto Shader::use() const -> void { glUseProgram(id); }

auto Shader::check_compile_errors(unsigned int shader, const std::string& type)
    -> void {
  int success;
  char infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
      glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
      printError("ERROR::SHADER_COMPILATION_ERROR of type: ", type, "\n",
                 infoLog);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (success == 0) {
      glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
      printError("ERROR::PROGRAM_LINKING_ERROR of type: ", type, "\n", infoLog);
    }
  }
}

auto Shader::set_int(const std::string& name, int value) const -> void {
  glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

auto Shader::set_mat4(const std::string& name, const glm::mat4& mat) const
    -> void {
  // glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE,
  //                    &mat[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(mat));
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
  auto draw(Shader& shader) -> void;
};

class Model {
 public:
  Model(const std::string& path);
  auto draw(Shader& shader) -> void;

 private:
  std::vector<Mesh> meshes;
  std::string directory;
  std::vector<Texture> textures_loaded;

  auto load_model(const std::string& path) -> void;
  auto process_node(aiNode* node, const aiScene* scene) -> void;
  static auto process_mesh(aiMesh* mesh, const aiScene* scene) -> Mesh;

  static auto load_material_textures(aiMaterial* mat, aiTextureType type,
                                     const std::string& type_name)
      -> std::vector<Texture>;

  static auto create_texture_from_file(const std::string& filename)
      -> unsigned int;
};

Model::Model(const std::string& path) { load_model(path); }

auto Model::load_model(const std::string& path) -> void {
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

  if ((scene == nullptr) ||
      ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) ||
      (scene->mRootNode == nullptr)) {
    printError("ERROR::ASSIMP::", importer.GetErrorString());
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));
  process_node(scene->mRootNode, scene);
}

auto Model::process_node(aiNode* node, const aiScene* scene) -> void {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}

auto Model::process_mesh(aiMesh* mesh, const aiScene* scene) -> Mesh {
  std::vector<Vertex> vertices;
  // std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Process vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.x = mesh->mVertices[i].x;
    vertex.y = mesh->mVertices[i].y;
    vertex.z = mesh->mVertices[i].z;

    if (mesh->mTextureCoords[0] != nullptr) {
      vertex.tex_u = mesh->mTextureCoords[0][i].x;
      vertex.tex_v = mesh->mTextureCoords[0][i].y;
    } else {
      vertex.tex_u = 0;
      vertex.tex_v = 0;
    }
    vertices.push_back(vertex);
  }

  // Process material
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Diffuse maps
    std::vector<Texture> diffuse_maps = load_material_textures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    // Specular maps
    std::vector<Texture> specular_maps = load_material_textures(
        material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
  }

  Mesh processed_mesh;
  processed_mesh.vertices = vertices;
  processed_mesh.textures = textures;
  processed_mesh.setup();
  return processed_mesh;
}

auto Model::load_material_textures(aiMaterial* mat, aiTextureType type,
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

auto Model::create_texture_from_file(const std::string& filename)
    -> unsigned int {
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

auto Mesh::setup() -> void {
  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);

  // Load Model
  stbi_set_flip_vertically_on_load(1);

  glNamedBufferStorage(vbo,
                       static_cast<long long>(vertices.size() * sizeof(Vertex)),
                       vertices.data(), 0);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

auto Mesh::draw(Shader& shader) -> void {
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;

  for (unsigned int i = 0; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);

    std::string number;
    std::string name = textures[i].type;
    if (name == "texture_diffuse") {
      number = std::to_string(diffuseNr++);
    } else if (name == "texture_specular") {
      number = std::to_string(specularNr++);
    }

    shader.set_int(name + number, static_cast<int>(i));
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

auto Model::draw(Shader& shader) -> void {
  for (auto& mesh : meshes) {
    mesh.draw(shader);
  }
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

  Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  shader.use();

  Model spider_model("models/spider.obj");

  // Projection Matrix
  const auto projection_mat =
      glm::perspective(glm::radians(45.0F), 1.3333F, 0.1F, 10.0F);
  shader.set_mat4("mProjection", projection_mat);

  // View Matrix
  const auto view_matrix =
      glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, -2.5F));
  shader.set_mat4("mView", view_matrix);

  // Set up buffers
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glEnable(GL_DEPTH_TEST);

  // Set up time calculations
  auto last_time = glfwGetTime();
  const auto y_rotation_speed = 0.5F;
  auto y_rotation = 0.0F;

  while (glfwWindowShouldClose(window) == 0) {
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

    // Model matrix
    const auto scale_matrix =
        glm::scale(glm::mat4(1.0), glm::vec3(0.005F, 0.005F, 0.005F));
    const auto rotate_y_matrix =
        glm::rotate(scale_matrix, y_rotation, glm::vec3(0.0F, 1.0F, 0.0F));
    const auto model_matrix = glm::mat4(rotate_y_matrix);
    shader.set_mat4("mModel", model_matrix);

    // Render the model
    spider_model.draw(shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // TODO: Add custom destructors to call glDeleteTextures(...),
  // glDeleteBuffers(...), glDeleteVertexArrays(...) and glDeleteShader(...)
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
