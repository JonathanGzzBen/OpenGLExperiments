#include "model.h"

#include <assimp/postprocess.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <iostream>
#include <ostream>

static auto get_image_data(const std::string& filename, int n_components)
    -> std::expected<
        std::tuple<std::unique_ptr<unsigned char, decltype(&stbi_image_free)>,
                   int, int>,
        std::string> {
  stbi_set_flip_vertically_on_load(true);
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

static auto get_texture(const std::string& filename)
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

model_loading::Model::Model(const char* path) { loadModel(path); }

auto model_loading::Model::Draw(const Program& program) const -> void {
  for (auto mesh : meshes) {
    mesh.Draw(program);
  }
}
auto model_loading::Model::loadModel(const std::string& path) -> void {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    std::println(std::cerr, "Could not open model file '{}'", path);
    return;
  }

  auto dirnameOf = [](const std::string& fname) -> std::string {
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : fname.substr(0, pos);
  };
  directory = dirnameOf(path);
  processNode(scene->mRootNode, scene);
}

auto model_loading::Model::processNode(aiNode* node, const aiScene* scene)
    -> void {
  for (size_t i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.emplace_back(processMesh(mesh, scene));
  }

  for (size_t i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}
auto model_loading::Model::processMesh(aiMesh* mesh, const aiScene* scene)
    -> Mesh {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  for (size_t i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.position.x = mesh->mVertices[i].x;
    vertex.position.y = mesh->mVertices[i].y;
    vertex.position.z = mesh->mVertices[i].z;

    vertex.normal.x = mesh->mNormals[i].x;
    vertex.normal.y = mesh->mNormals[i].y;
    vertex.normal.z = mesh->mNormals[i].z;

    if (mesh->mTextureCoords[0]) {
      vertex.uv.x = mesh->mTextureCoords[0][i].x;
      vertex.uv.y = mesh->mTextureCoords[0][i].y;
    } else {
      vertex.uv = glm::vec2(0.0f, 0.0f);
    }
    vertices.emplace_back(vertex);
  }

  for (size_t i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (size_t j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuseMaps = loadMaterialTextures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(
        material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }

  return Mesh(vertices, indices, textures);
}
auto model_loading::Model::loadMaterialTextures(aiMaterial* mat,
                                                aiTextureType type,
                                                std::string typeName)
    -> std::vector<Texture> {
  std::vector<Texture> textures;
  for (size_t i = 0; i < mat->GetTextureCount(type); i++) {
    aiString texturePath;
    mat->GetTexture(type, i, &texturePath);
    bool skip = false;
    for (size_t j = 0; j < textures_loaded.size(); j++) {
      if (std::strcmp(textures_loaded[j].path.c_str(), texturePath.C_Str()) ==
          0) {
        textures.emplace_back(textures_loaded[j]);
        skip = true;
        break;
      }
    }
    if (!skip) {
      Texture texture;
      auto tex_id = get_texture(directory + "/" + texturePath.C_Str());
      if (!tex_id) {
        std::println(std::cerr, "Could not load texture");
        continue;
      }
      texture.id = tex_id.value();
      texture.type = typeName;
      texture.path = texturePath.C_Str();
      textures.emplace_back(texture);
    }
  }
  return textures;
}
