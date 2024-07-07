#pragma once

#include <string>
#include <vector>
#include <memory>

struct aiScene;
struct aiAnimation;

namespace animation {
    struct RigContainer;
    struct Animation;

    class AnimationLoader {
    public:
        AnimationLoader();
        ~AnimationLoader();

        void loadAnimations(
            animation::RigContainer& rig,
            const std::string& uniquePrefix,
            const std::string& filePath);

        void loadAnimations(
            animation::RigContainer& rig,
            const std::string& uniquePrefix,
            const std::string& filePath,
            const aiScene* scene);

    private:
        std::unique_ptr<animation::Animation> loadAnimation(
            animation::RigContainer& rig,
            int16_t animIndex,
            const std::string& uniquePrefix,
            const aiScene* scene,
            const aiAnimation* anim);
    };
}
