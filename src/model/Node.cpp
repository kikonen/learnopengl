#include "Node.h"

#include <mutex>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "controller/NodeController.h"


namespace {
    int objectIDbase = 100;

    std::mutex object_id_lock;
}


int Node::nextID()
{
    std::lock_guard<std::mutex> lock(object_id_lock);
    return ++objectIDbase;
}

Node::Node(std::shared_ptr<NodeType> type)
    : type(type),
    objectID(nextID())
{
}

Node::~Node()
{
    KI_INFO_SB("NODE: delete type=" << type->typeID << " objectId=" << objectID);
}

void Node::prepare(const Assets& assets)
{
    if (controller) {
        controller->prepare(assets, *this);
    }
}

bool Node::update(const RenderContext& ctx)
{
    if (!controller) return false;
    return controller->update(ctx, *this);
}

void Node::bind(const RenderContext& ctx, Shader* shader)
{
    updateModelMatrix();
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch)
{
    updateModelMatrix();
    if (type->flags.mirror) {
        int x = 0;
    }
    batch.add(modelMat, normalMat, objectID);
}

void Node::draw(const RenderContext& ctx)
{
    // NOTE KI shader side supports *ONLY* instanced rendering
    singleBatch.batchSize = 1;
    singleBatch.prepare(type.get());

    singleBatch.draw(ctx, this, type->boundShader);
    //type->mesh->draw(ctx);
}

void Node::updateModelMatrix() {
    bool dirtyModel = dirtyRot || dirtyTrans || dirtyScale;

    if (!dirtyModel) {
        return;
    }


    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    if (dirtyRot) {
        rotMat = glm::toMat4(glm::quat(glm::radians(rotation)));
        dirtyRot = false;
    }

    if (dirtyTrans) {
        transMat = glm::translate(
            glm::mat4(1.0f),
            pos
        );
        dirtyTrans = false;
    }

    if (dirtyScale) {
        scaleMat = glm::scale(
            glm::mat4(1.0f),
            scale
        );
        dirtyScale = false;
    }

    if (dirtyModel) {
        modelMat = transMat * rotMat * scaleMat;

        // https://learnopengl.com/Lighting/Basic-Lighting
        // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
        // normal = mat3(transpose(inverse(model))) * aNormal;
        normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
    }
}

void Node::setPos(const glm::vec3& pos) {
    this->pos = pos;
    dirtyTrans = true;
}

const glm::vec3&  const Node::getPos() {
    return pos;
}

void Node::setRotation(const glm::vec3& rotation) {
    this->rotation = rotation;
    dirtyRot = true;
}

const glm::vec3&  Node::getRotation() {
    return rotation;
}

void Node::setScale(float scale) {
    this->scale.x = scale;
    this->scale.y = scale;
    this->scale.z = scale;
    dirtyScale = true;
}

void Node::setScale(const glm::vec3& scale)
{
    this->scale = scale;
    dirtyScale = true;
}

const glm::vec3& Node::getScale() {
    return scale;
}
