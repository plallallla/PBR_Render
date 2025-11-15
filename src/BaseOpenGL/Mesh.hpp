#pragma once
#include <OpenGL/gltypes.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <cstddef>
#include <vector>
#include "Buffer.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include "ShaderProgram.hpp"

class Mesh
{
    VertexArray _va;
    size_t _elements_ct;
    std::vector<Texture> _textures;
public:
    Mesh(Mesh&& other) noexcept : _va{other._va}, _elements_ct{other._elements_ct}, _textures{other._textures}
    {
        other._va._id = 0;
    }

    Mesh(aiMesh* mesh, const aiScene* scene, std::string_view directory)
    {
        // vertices
        std::vector<float> vertices;
        bool has_tangent = mesh->mTangents && mesh->mBitangents;
        int vertex_rank = has_tangent ? 14 : 8;
        vertices.reserve(mesh->mNumVertices * vertex_rank);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vertices.insert(vertices.end(), 
            {
                // vertex
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z,
                // normal
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            });
            // texture uv
            if (mesh->mTextureCoords[0]) 
            {
                vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
            }
            else 
            {
                vertices.insert(vertices.end(), {0, 0});
            }
            if (has_tangent)
            {
                vertices.insert(vertices.end(), 
                {
                    // tangent
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z,
                    // bitangent
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z
                });
            }
        }
        GLuint vb = BUFFER.generate_buffer(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data());
        // indices
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        GLuint eb = BUFFER.generate_buffer(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data());
        _va.attach_buffer(has_tangent ? PNTTB_LAYOUT : PNT_LAYOUT, vb, eb);
        _elements_ct = indices.size();
        if (mesh->mMaterialIndex >= 0)
        {
            TEXTURE_MANAGER.load_from_material(scene->mMaterials[mesh->mMaterialIndex], _textures, directory);
        }
    }

    void attach_extra_buffer(const BufferLayout& layout, GLuint buffer_id)
    {
        _va.attach_vertex_buffer(layout, buffer_id);
    }

    void render_elements(const ShaderProgram& shader) const
    {
        shader.use();
        for (int i = 0; i < _textures.size(); i++)
        {
            shader.active_sampler(i, _textures[i]);
        }
        _va.bind();
        glDrawElements(GL_TRIANGLES, _elements_ct, GL_UNSIGNED_INT, 0);
        _va.unbind();
    }

    void render_elements_instanced(const ShaderProgram& shader, GLsizei instance_count) const
    {
        shader.use();
        for (int i = 0; i < _textures.size(); i++)
        {
            shader.active_sampler(i, _textures[i]);
        }
        _va.bind();
        glDrawElementsInstanced(GL_TRIANGLES, _elements_ct, GL_UNSIGNED_INT, 0, instance_count);
        _va.unbind();
    }
};
