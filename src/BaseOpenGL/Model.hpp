#pragma once
#include <OpenGL/gltypes.h>
#include <string>
#include <vector>
#include "Mesh.hpp"

class Model
{
    std::string _directory;
    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            _meshes.emplace_back(mesh, scene, _directory);
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

  public:
    std::vector<Mesh> _meshes;
    Model(std::string path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG.info("ERROR::ASSIMP::" + std::string{importer.GetErrorString()});
            return;
        }
        _directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }
    void attach_extra_buffer(const BufferLayout& layout, GLuint buffer_id)
    {
        for (unsigned int i = 0; i < _meshes.size(); i++)
        {
            _meshes[i].attach_extra_buffer(layout, buffer_id);
        }
    }
    void render_elements(const ShaderProgram& shader)
    {
        for (unsigned int i = 0; i < _meshes.size(); i++)
        {
            _meshes[i].render_elements(shader);
        }
    }
    void render_elements_instanced(const ShaderProgram& shader, GLsizei instance_count) const
    {
        for (unsigned int i = 0; i < _meshes.size(); i++)
        {
            _meshes[i].render_elements_instanced(shader, instance_count);
        }
    }
};
