#pragma comment(lib, "avrt.lib")

#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>

#include <fmt/format.h>

#include "util/Ref.h"

#include "asset/Assets.h"
#include "engine/AssetsLoader.h"

#include "mesh/MeshSet.h"
#include "mesh_set/AssimpImporter.h"

#include "mesh_set/encoder/MeshSetEncoder.h"
#include "mesh_set/encoder/ModelMeshEncoder.h"

#include "mesh/ModelMesh.h"

#include "material/Material.h"

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

    const std::string& rootDir = assets.rootDir;
    bool smoothNormals = true;
    bool forceNormals = true;
    bool assimpDebug = assets.assimpDebug;

    auto meshSet = util::Ref<mesh::MeshSet>::create(
        "1",
        rootDir,
        meshPath,
        smoothNormals,
        forceNormals);

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

    std::shared_ptr<std::atomic_bool> alive = std::make_shared<std::atomic_bool>(true);
    std::unique_ptr<mesh_set::MeshSetImporter> importer;
    importer = std::make_unique<mesh_set::AssimpImporter>(alive, assimpDebug);

    auto loaded = importer->load(*meshSet, material, false);
    //std::cout << fmt::format("loaded: {}\n", loaded);

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

    std::cout << fmt::format("mesh_set: {}", meshSet->m_name);

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

    const std::string inputFile = argv[1];
    const std::string outputPath = argv[2];

    std::cout << fmt::format("input: {}\n", inputFile);
    std::cout << fmt::format("output: {}\n", outputPath);

    Assets::set(loadAssets());

    auto meshSet = loadMeshSet(inputFile);
    saveMeshSet(meshSet, outputPath);

    if (0) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }
}
