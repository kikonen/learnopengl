#pragma once

struct aiScene;
struct aiNode;

namespace mesh_set
{
    class SceneMetaData
    {
    public:
        SceneMetaData();

        void dumpMetaData(
            const aiScene* scene);

    private:
        void dumpMetaData(
            const aiScene* scene,
            const aiNode* node,
            int level);
    };
}
