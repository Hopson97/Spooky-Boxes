#pragma once

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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

  public:
    struct ModelMesh
    {
        BasicMesh mesh;
        std::vector<int> textures;

        bool buffered = false;
    };

    Model() = default;
    Model(const BasicMesh& mesh);
    Model(const std::filesystem::path& path);

    bool load_from_file(const std::filesystem::path& path);
    void draw(Shader& shader);
    const std::vector<ModelMesh>& get_meshes() const;

  private:
    void process_node(aiNode* node, const aiScene* scene);
    ModelMesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<size_t> load_material(aiMaterial* material, aiTextureType texture_type);

    std::vector<ModelMesh> meshes_;
    std::vector<Texture> textures_cache_;
    std::string directory_;
};