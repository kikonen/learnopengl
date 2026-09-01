#pragma comment(lib, "avrt.lib")

#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>

#include <fmt/format.h>

#include "util/Ref.h"
#include "util/file.h"

#include "asset/Assets.h"
#include "engine/AssetsLoader.h"
#include "engine/SystemInit.h"

#include "mesh/MeshSet.h"
#include "mesh_set/AssimpImporter.h"
#include "mesh_set/AnimationImporter.h"

#include "mesh_set/encoder/MeshSetEncoder.h"
#include "mesh_set/encoder/ModelMeshEncoder.h"

#include "mesh/ModelMesh.h"

#include "material/Material.h"

#include "animation/AnimationPath.h"

#include "engine/Engine.h"
#include "registry/Registry.h"

#include "loader/converter/YamlConverter.h"
#include "loader/document.h"
#include "loader/Context.h"
#include "loader/Loaders.h"
#include "loader/MeshLoader.h"
#include "loader/MaterialLoader.h"

Assets loadAssets()
{
    AssetsLoader loader{ "scene/assets.yml" };
    return loader.load();
}

util::Ref<mesh::MeshSet> loadMeshSet(
    const std::string& meshPath)
{
    const auto& assets = Assets::get();

    //auto future = MeshSetRegistry::get().getMeshSet(
    //    meshData.id,
    //    assets.modelsDir,
    //    meshData.path,
    //    meshData.smoothNormals,
    //    meshData.forceNormals,
    //    meshData.getAnimationPaths());

    loader::MeshData meshData;
    {

        loader::YamlConverter converter;
        const auto& currDir = util::dirName(meshPath);
        auto doc = converter.load(meshPath);

        Engine engine;
        auto registry = util::Ref<Registry>::create(engine);
        auto ctx = util::Ref<loader::Context>::create(assets.sceneDir, "na");
        loader::Loaders loaders{ ctx };
        loaders.prepare(registry);

        std::vector<animation::AnimationPath> animationPaths;
        loaders.m_meshLoader.loadMesh(
            doc.findNode("mesh"),
            currDir,
            meshData,
            loaders);
    }

    auto meshSet = util::Ref<mesh::MeshSet>::create(
        meshData.id,
        assets.modelsDir,
        meshData.path,
        meshData.smoothNormals,
        meshData.forceNormals,
        meshData.getAnimationPaths());

    auto material = util::Ref<Material>::create();
    {
        material = Material::createMaterial(BasicMaterial::gold);
        material->addTexture(TextureType::diffuse, "foo", false);
        material->addTexture(TextureType::map_normal, "foo_normal", false);
        material->m_programNames.insert({ MaterialProgramType::shader, "g_tex" });
        material->m_programNames.insert({ MaterialProgramType::shadow, "shadow" });
        //MaterialData data;
        //MaterialLoader loader;
        //loader.loadMaterial(data);
    }

    bool loaded;
    {
        bool assimpDebug = assets.assimpDebug;
        std::shared_ptr<std::atomic_bool> alive = std::make_shared<std::atomic_bool>(true);
        std::unique_ptr<mesh_set::MeshSetImporter> importer;
        importer = std::make_unique<mesh_set::AssimpImporter>(alive, assimpDebug);

        loaded = importer->load(*meshSet, material, false);
        //std::cout << fmt::format("loaded: {}\n", loaded);
    }

    {
        for (const auto& animationPath : meshSet->getAnimationPaths()) {
            // resolve path
            std::string filePath;
            {
                {
                    filePath = util::joinPathExt(
                        meshSet->getRootDir(),
                        meshSet->getDir(),
                        animationPath.path, "");
                }

                if (!util::fileExists(filePath)) {
                    filePath = util::joinPath(
                        meshSet->getRootDir(),
                        animationPath.path);
                }
            }

            for (auto& mesh : meshSet->getMeshes()) {
                const auto& rig = mesh->getRig();
                if (!rig) continue;

                mesh_set::AnimationImporter importer{};
                importer.loadAnimations(
                    *rig,
                    animationPath.animationPrefix,
                    filePath);
            }
        }
    }

    return meshSet;
}

void saveMeshSet(
    const util::Ref<mesh::MeshSet>& meshSet,
    const std::string& outputPath)
{
    if (meshSet->empty()) return;

    YAML::Emitter out;
    out.SetIndent(4);

    mesh_set::encoder::MeshSetEncoder encoder;
    encoder.encode(out, meshSet);

    std::cout << fmt::format("mesh_set: {}", meshSet->getName());

    {
        std::ofstream fout(outputPath);
        fout << out.c_str();
        fout << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: mesh_preprocessor <inputmesh> <outputmesh>" << std::endl;
        return EXIT_FAILURE;
    }

    Log::init("log/mesh_preprocessor.log");

    const std::string inputFile = argv[1];
    const std::string outputPath = argv[2];

    std::cout << fmt::format("input: {}\n", inputFile);
    std::cout << fmt::format("output: {}\n", outputPath);

    SystemInit::init();
    Assets::set(loadAssets());

    auto meshSet = loadMeshSet(inputFile);
    saveMeshSet(meshSet, outputPath);

    SystemInit::release();

    if (0) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }
}
