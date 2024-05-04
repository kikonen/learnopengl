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
            const std::string& namePrefix,
            const std::string& filePath);

        void loadAnimations(
            animation::RigContainer& rig,
            const std::string& namePrefix,
            const aiScene* scene);

    private:
        std::unique_ptr<animation::Animation> loadAnimation(
            animation::RigContainer& rig,
            const std::string& namePrefix,
            const aiScene* scene,
            const aiAnimation* anim);
    };
}
