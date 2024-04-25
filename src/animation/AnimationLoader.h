#pragma once

#include <vector>
#include <memory>

struct aiScene;
struct aiAnimation;

namespace animation {
    struct RigContainer;
    struct Animation;

    class AnimationLoader {
        ~AnimationLoader();

        std::vector<std::unique_ptr<animation::Animation>> loadAnimations(
            animation::RigContainer& rig,
            aiScene* scene);

    private:
        std::unique_ptr<animation::Animation> loadAnimation(
            animation::RigContainer& rig,
            const aiScene* scene,
            const aiAnimation* anim);
    };
}
