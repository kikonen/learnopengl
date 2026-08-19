#include "RigEncoder.h"

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/ClipContainer.h"
#include "animation/Animation.h"
#include "animation/Clip.h"

namespace mesh_set::encoder
{
    void encodeNodes(
        YAML::Emitter& out,
        const std::string& key,
        const std::vector<animation::RigNode>& nodes,
        const std::function<void(YAML::Emitter&, const animation::RigNode&)>& fn
    )
    {
        out << YAML::Key << key;
        out << YAML::Value << YAML::BeginSeq;
        for (const auto& node : nodes) {
            fn(out, node);
        }
        out << YAML::EndSeq;
    }

    void encodeAnimations(
        YAML::Emitter& out,
        const std::string& key,
        const animation::ClipContainer& clipContainer
    )
    {
        out << YAML::Key << key;
        out << YAML::Value << YAML::BeginSeq;
        for (const auto& animPtr : clipContainer.m_animations) {
            const auto& anim = *animPtr;

            out << YAML::BeginMap;
            {
                out << YAML::Key << "name";
                out << YAML::Value << anim.m_name;

                out << YAML::Key << "unique_name";
                out << YAML::Value << anim.m_uniqueName;

                out << YAML::Key << "duration";
                out << YAML::Value << anim.m_duration;

                out << YAML::Key << "ticks_per_second";
                out << YAML::Value << anim.m_ticksPerSecond;

                out << YAML::Key << "clip_count";
                out << YAML::Value << anim.getClipCount();
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    void encodeClips(
        YAML::Emitter& out,
        const std::string& key,
        const animation::ClipContainer& clipContainer
    )
    {
        out << YAML::Key << key;
        out << YAML::Value << YAML::BeginSeq;
        for (auto& clip : clipContainer.m_clips) {
            out << YAML::BeginMap;
            {
                //out << YAML::Key << "id";
                //out << YAML::Value << clip.m_id;

                out << YAML::Key << "unique_name";
                out << YAML::Value << clip.m_uniqueName;

                //out << YAML::Key << "clip";
                //out << YAML::Value << clip.m_animationClip;

                out << YAML::Key << "animation";
                out << YAML::Value << clip.m_animationName;

                out << YAML::Key << "first_frame";
                out << YAML::Value << clip.m_firstFrame;

                out << YAML::Key << "last_frame";
                out << YAML::Value << clip.m_lastFrame;

                out << YAML::Key << "first_tick";
                out << YAML::Value << clip.m_firstTick;

                out << YAML::Key << "last_tick";
                out << YAML::Value << clip.m_lastTick;

                out << YAML::Key << "duration";
                out << YAML::Value << clip.m_duration;

                out << YAML::Key << "duration_secs";
                out << YAML::Value << clip.m_durationSecs;

                out << YAML::Key << "loop";
                out << YAML::Value << clip.m_loop;

                out << YAML::Key << "single";
                out << YAML::Value << clip.m_single;
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    void encodeClipLUTs(
        YAML::Emitter& out,
        const std::string& key,
        const util::Ref<animation::Rig>& rig,
        const animation::ClipContainer& clipContainer
    )
    {
        out << YAML::Key << key;
        out << YAML::Value << YAML::BeginSeq;
        for (int clipIndex = -1; auto& channelLUTs : clipContainer.getClipLUTs()) {
            clipIndex++;
            const auto& clip = clipContainer.m_clips[clipIndex];

            out << YAML::BeginMap;
            {
                out << YAML::Key << "unique_name";
                out << YAML::Value << clip.m_uniqueName;

                out << YAML::Key << "channel_count";
                out << YAML::Value << channelLUTs.size();

                {
                    out << YAML::Key << "channels";
                    out << YAML::Value << YAML::BeginSeq;
                    for (int channelIndex = -1;  const auto& channelLUT : channelLUTs) {
                        channelIndex++;

                        out << YAML::BeginMap;
                        {
                            const auto* node = rig->getNode(channelIndex);

                            out << YAML::Key << "node_index";
                            out << YAML::Value << channelIndex;

                            out << YAML::Key << "node";
                            out << YAML::Value << node->m_name;

                            out << YAML::Key << "size";
                            out << YAML::Value << channelLUT.getPositions().size();

                            {
                                out << YAML::Key << "positions";
                                {
                                    std::vector<float> values;
                                    values.reserve(channelLUT.getPositions().size() * 3);
                                    for (const auto& pos : channelLUT.getPositions()) {
                                        encodeVec3(values, pos);
                                    }
                                    out << YAML::Value;
                                    encodeCompressed(out, values);
                                }
                            }
                            {
                                out << YAML::Key << "rotations";
                                {
                                    std::vector<float> values;
                                    values.reserve(channelLUT.getRotations().size() * 3);
                                    for (const auto& pos : channelLUT.getRotations()) {
                                        encodeQuat(values, pos);
                                    }
                                    out << YAML::Value;
                                    encodeCompressed(out, values);
                                }
                            }
                            {
                                out << YAML::Key << "scales";
                                {
                                    std::vector<float> values;
                                    values.reserve(channelLUT.getScales().size() * 3);
                                    for (const auto& pos : channelLUT.getScales()) {
                                        encodeVec3(values, pos);
                                    }
                                    out << YAML::Value;
                                    encodeCompressed(out, values);
                                }
                            }

                            out << YAML::Key << "inv_scale_factor";
                            out << YAML::Value << channelLUT.getInvScaleFactor();
                        }
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    RigEncoder::RigEncoder() = default;
    RigEncoder::~RigEncoder() = default;

    void RigEncoder::encode(
        YAML::Emitter& out,
        const util::Ref<animation::Rig>& rig)
    {
        out << YAML::BeginMap;
        {
            out << YAML::Key << "name";
            out << YAML::Value << rig->getName();

            out << YAML::Key << "alias";
            out << YAML::Value << rig->getAlias();

            //out << YAML::Key << "root";
            //out << YAML::Value << rig->m_skeletonRootNodeName;

            //out << YAML::Key << "vertex_count";
            //out << YAML::Value << rig->m_vertices.size();
        }
        {
            out << YAML::Key << "node_count";
            out << YAML::Value << rig->getNodes().size();

            encodeNodes(
                out,
                "nodes",
                rig->getNodes(),
                [](YAML::Emitter& out, const animation::RigNode& node) {
                out << YAML::BeginMap;
                {
                    out << YAML::Key << "name";
                    out << YAML::Value << node.m_name;

                    out << YAML::Key << "parent";
                    out << YAML::Value << node.m_parentIndex;

                    out << YAML::Key << "transform";
                    encodeMat4(out, node.m_transform);
                }
                out << YAML::EndMap;
            });
        }
        {
            encodeAnimations(
                out,
                "animations",
                rig->getClipContainer()
            );
        }
        {
            encodeClips(
                out,
                "clips",
                rig->getClipContainer()
            );
        }
        {
            encodeClipLUTs(
                out,
                "clip_luts",
                rig,
                rig->getClipContainer()
            );
        }
        out << YAML::EndMap;
    }
}
