#ifndef MODEL_H
#define MODEL_H

#include <assimp/scene.h>

#include <vector>

#include "mesh.h"

namespace model_loading {

class Model {
 public:
  explicit Model(const char *path);
  auto Draw(const Program &program) const -> void;

 private:
  std::vector<Mesh> meshes;
  std::string directory;
  std::vector<Texture> textures_loaded;

  auto loadModel(const std::string &path) -> void;
  auto processNode(aiNode *node, const aiScene *scene) -> void;
  auto processMesh(aiMesh *mesh, const aiScene *scene) -> Mesh;
  auto loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                            std::string typeName) -> std::vector<Texture>;
};

}  // namespace model_loading

#endif  // MODEL_H
