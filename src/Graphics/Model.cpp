#include "Model.h"

#include <assimp/Importer.hpp>

#include "OpenGL/Shader.h"
#include <iostream>

Model::Model(const std::filesystem::path& path)
{
    load_from_file(path);
}

bool Model::load_from_file(const std::filesystem::path& path)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs |
                                                      aiProcess_GenNormals);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cerr << "Could not load model " << path << "\n"
                  << importer.GetErrorString() << '\n';
        return false;
    }

    directory_ = path.string().substr(0, path.string().find_last_of('/'));

    process_node(scene->mRootNode, scene);
    return true;
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
    {
        auto mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(process_mesh(mesh, scene));
    }

    for (unsigned i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

std::vector<int> Model::load_material(aiMaterial* material, aiTextureType texture_type)
{
    std::vector<int> textures;

    for (unsigned i = 0; i < material->GetTextureCount(texture_type); i++)
    {
        aiString str;
        material->GetTexture(texture_type, i, &str);

        // Check if the texture is already in the cache
        bool should_load = true;
        for (int i = 0; i < textures_cache_.size(); i++)
        {
            if (textures_cache_[i].path == std::string(str.C_Str()))
            {
                textures.push_back(i);
                should_load = false;
                break;
            }
        }

        // Load the texture if it is not
        if (should_load)
        {
            Texture texture;

            texture.type = [texture_type]()
            {
                switch (texture_type)
                {

                    case aiTextureType_DIFFUSE:
                        return "diffuse";
                        break;
                    case aiTextureType_SPECULAR:
                        return "specular";
                        break;
                    default:
                        return "Unknown";
                }
            }();

            texture.path = str.C_Str();
            texture.texture.load_from_file(directory_ + "/" + str.C_Str(), 1, true, false);
            textures.push_back(textures_cache_.size());
            textures_cache_.push_back(std::move(texture));
        }
    }

    return textures;
}

Model::ModelMesh Model::process_mesh(aiMesh* ai_mesh, const aiScene* scene)
{
    ModelMesh mesh;

    // Process the Assimp's mesh vertices
    for (unsigned i = 0; i < ai_mesh->mNumVertices; i++)
    {
        BasicVertex v;

        v.position.x = ai_mesh->mVertices[i].x;
        v.position.y = ai_mesh->mVertices[i].y;
        v.position.z = ai_mesh->mVertices[i].z;

        if (ai_mesh->HasNormals())
        {

            v.normal.x = ai_mesh->mNormals[i].x;
            v.normal.y = ai_mesh->mNormals[i].y;
            v.normal.z = ai_mesh->mNormals[i].z;
        }
        if (ai_mesh->mTextureCoords[0])
        {
            v.texture_coord.x = ai_mesh->mTextureCoords[0][i].x;
            v.texture_coord.y = ai_mesh->mTextureCoords[0][i].y;
        }
        else
        {
            v.texture_coord = {0.0, 0.0};
        }

        mesh.mesh.vertices.push_back(v);
    }

    // Process Indices
    for (unsigned i = 0; i < ai_mesh->mNumFaces; i++)
    {
        auto face = ai_mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++)
        {
            mesh.mesh.indices.push_back(face.mIndices[j]);
        }
    }

    if (ai_mesh->mMaterialIndex >= 0)
    {
        auto material = scene->mMaterials[ai_mesh->mMaterialIndex];
        auto diffuse_maps = load_material(material, aiTextureType_DIFFUSE);
        auto specular_maps = load_material(material, aiTextureType_SPECULAR);

        mesh.textures.insert(mesh.textures.end(), diffuse_maps.begin(), diffuse_maps.end());
        mesh.textures.insert(mesh.textures.end(), specular_maps.begin(), specular_maps.end());
    }

    return mesh;
}

void Model::draw(Shader& shader)
{
    for (ModelMesh& mesh : meshes_)
    {
        if (!mesh.buffered)
        {
            std::cout << "Buffering mesh!\n";
            mesh.mesh.buffer();
            mesh.buffered = true;
        }

        GLuint diffuse_id = 0;
        GLuint specular_id = 0;
        for (int i = 0; i < mesh.textures.size(); i++)
        {
            std::string number;
            std::string name = textures_cache_[mesh.textures[i]].type;
            if (name == "diffuse")
                number = std::to_string(diffuse_id++);
            else if (name == "specular")
                number = std::to_string(specular_id++);

            auto uni = "material." + name + number;
            shader.set_uniform(uni, i);

            textures_cache_[mesh.textures[i]].texture.bind(i);
        }
        // draw mesh
        mesh.mesh.bind();
        mesh.mesh.draw();
    }
}