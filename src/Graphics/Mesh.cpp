#include "Mesh.h"

#include <numeric>

#include <SFML/Graphics/Image.hpp>

#include "../Utils/HeightMap.h"

/*
Cool blue RGB:

{0.2f, 0.2f, 0.5f},
{0.2f, 0.5f, 0.5f},
{0.2f, 0.5f, 1.0f},
{0.2f, 0.5f, 0.5f},
*/

BasicMesh generate_quad_mesh(float w, float h)
{
    BasicMesh mesh;
    mesh.vertices = {{{w, h, 0.0f}, {0.0f, 1.0f}, {0, 0, 1}},
                     {{0, h, 0.0f}, {1.0f, 1.0f}, {0, 0, 1}},
                     {{0, 0, 0.0f}, {1.0f, 0.0f}, {0, 0, 1}},
                     {{w, 0, 0.0f}, {0.0f, 0.0f}, {0, 0, 1}}

    };

    mesh.indices = {0, 1, 2, 2, 3, 0};
    mesh.buffer();
    return mesh;
}

BasicMesh generate_plane_mesh(float w, float d)
{
    BasicMesh mesh;
    mesh.vertices = {{{w, 0, d}, {0.0f, 1.0f}, {0, 1, 0}},
                     {{0, 0, d}, {1.0f, 1.0f}, {0, 1, 0}},
                     {{0, 0, 0.0f}, {1.0f, 0.0f}, {0, 1, 0}},
                     {{w, 0, 0.0f}, {0.0f, 0.0f}, {0, 1, 0}}

    };

    mesh.indices = {2, 3, 0, 0, 1, 2};
    mesh.buffer();
    return mesh;
}

BasicMesh generate_cube_mesh(const glm::vec3& dimensions, bool repeat_texture)
{
    BasicMesh mesh;

    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;

    float txrx = repeat_texture ? w : 1.0f;
    float txry = repeat_texture ? h : 1.0f;

    // clang-format off
    mesh.vertices = {
        {{w, h, d}, {txrx, 0.0f}, {0.0f, 0.0f, 1.0f}},  
        {{0, h, d}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{0, 0, d}, {0.0f, txry}, {0.0f, 0.0f, 1.0f}},  
        {{w, 0, d}, {txrx, txry}, {0.0f, 0.0f, 1.0f}},

        {{0, h, d}, {txrx, 0.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{0, h, 0}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{0, 0, 0}, {0.0f, txry}, {-1.0f, 0.0f, 0.0f}}, 
        {{0, 0, d}, {txrx, txry}, {-1.0f, 0.0f, 0.0f}},

        {{0, h, 0}, {txrx, 0.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{w, h, 0}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{w, 0, 0}, {0.0f, txry}, {0.0f, 0.0f, -1.0f}}, 
        {{0, 0, 0}, {txrx, txry}, {0.0f, 0.0f, -1.0f}},

        {{w, h, 0}, {txrx, 0.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, h, d}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{w, 0, d}, {0.0f, txry}, {1.0f, 0.0f, 0.0f}},  
        {{w, 0, 0}, {txrx, txry}, {1.0f, 0.0f, 0.0f}},

        {{w, h, 0}, {txrx, 0.0f}, {0.0f, 1.0f, 0.0f}},  
        {{0, h, 0}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0, h, d}, {0.0f, txry}, {0.0f, 1.0f, 0.0f}},  
        {{w, h, d}, {txrx, txry}, {0.0f, 1.0f, 0.0f}},

        {{0, 0, 0}, {txrx, 0.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{w, 0, 0}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{w, 0, d}, {0.0f, txry}, {0.0f, -1.0f, 0.0f}}, 
        {{0, 0, d}, {txrx, txry}, {0.0f, -1.0f, 0.0f}},
    };
    // clang-format on

    int index = 0;
    for (int i = 0; i < 6; i++)
    {
        mesh.indices.push_back(index);
        mesh.indices.push_back(index + 1);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 3);
        mesh.indices.push_back(index);
        index += 4;
    }
    mesh.buffer();

    return mesh;
}

BasicMesh generate_centered_cube_mesh(const glm::vec3& dimensions)
{
    BasicMesh mesh;

    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;

    // clang-format off
    mesh.vertices = {
        {{w, h, d}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  
        {{-w, h, d}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-w, -h, d}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  
        {{w, -h, d}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        {{-w, h, d}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{-w, h, -d}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-w, -h, -d}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}}, 
        {{-w, -h, d}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

        {{-w, h, -d}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{w, h, -d}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{w, -h, -d}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}, 
        {{-w, -h, -d}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},

        {{w, h, -d}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, h, d}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{w, -h, d}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  
        {{w, -h, -d}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

        {{w, h, -d}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  
        {{-w, h, -d}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-w, h, d}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  
        {{w, h, d}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

        {{-w, -h, -d}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{w, -h, -d}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
        {{w, -h, d}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}}, 
        {{-w, -h, d}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
    };
    // clang-format on

    int index = 0;
    for (int i = 0; i < 6; i++)
    {
        mesh.indices.push_back(index);
        mesh.indices.push_back(index + 1);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 3);
        mesh.indices.push_back(index);
        index += 4;
    }
    mesh.buffer();

    return mesh;
}

BasicMesh generate_terrain_mesh(const HeightMap& height_map)
{
    BasicMesh mesh;
    update_terrain_mesh(mesh, height_map);
    mesh.buffer();
    return mesh;
}

void update_terrain_mesh(BasicMesh& mesh, const HeightMap& height_map)
{
    float sizef = static_cast<float>(height_map.size);

    mesh.vertices.clear();
    mesh.indices.clear();

    for (int z = 0; z < height_map.size; z++)
    {
        for (int x = 0; x < height_map.size; x++)
        {
            GLfloat fz = static_cast<GLfloat>(z);
            GLfloat fx = static_cast<GLfloat>(x);

            BasicVertex vertex;
            vertex.position.x = fx;
            vertex.position.y = height_map.get_height(x, z);
            vertex.position.z = fz;

            vertex.texture_coord.s = fx;
            vertex.texture_coord.t = fz;

            float height_left = x > 0 ? height_map.get_height(x - 1, z) : 0;
            float height_right = x < height_map.size - 1 ? height_map.get_height(x + 1, z) : 0;
            float height_down = z > 0 ? height_map.get_height(x, z - 1) : 0;
            float height_up = z < height_map.size - 1 ? height_map.get_height(x, z + 1) : 0;

            vertex.normal = glm::normalize(glm::vec3{
                height_left - height_right,
                2.0f,
                height_down - height_up,
            });

            mesh.vertices.push_back(vertex);
        }
    }

    for (int z = 0; z < height_map.size - 1; z++)
    {
        for (int x = 0; x < height_map.size - 1; x++)
        {
            int topLeft = (z * height_map.size) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * height_map.size) + x;
            int bottomRight = bottomLeft + 1;

            mesh.indices.push_back(topLeft);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(bottomRight);
        }
    }
}
