#pragma once

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "OpenGL/Texture.h"

class Shader;

class Model
{
    struct Texture
    {
        Texture2D texture;
        std::string type;
        std::string path;
    };

    struct ModelMesh
    {
        BasicMesh mesh;
        std::vector<int> textures;

        bool buffered = false;
    };

  public:
    Model() = default;

    Model(const std::filesystem::path& path);
    bool load_from_file(const std::filesystem::path& path);

    Model(const BasicMesh& mesh);

    void draw(Shader& shader);

  private:
    void process_node(aiNode* node, const aiScene* scene);
    ModelMesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<int> load_material(aiMaterial* material, aiTextureType texture_type);


    std::vector<ModelMesh> meshes_;

    std::vector<Texture> textures_cache_;

    std::string directory_;
};