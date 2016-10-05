/**
 * @file   SceneRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.28
 *
 * @brief  Implementation of the scene renderable object.
 */

#include "SceneRenderable.h"
#include "gfx/ArcballCamera.h"
#include "gfx/glrenderer/MeshRenderable.h"
#include <glm/glm.hpp>
#include <core/serializationHelper.h>

namespace cgu {

    SceneRenderable::SceneRenderable(MeshRenderable* theRenderable, const glm::vec3& pos, const glm::quat& orient) :
        renderable_(theRenderable),
        position_(pos),
        orientation_(orient),
        worldMatrix_(1.0f)
    {
        UpdatePositionOrientation(position_, orientation_);
    }

    SceneRenderable::~SceneRenderable() = default;

    SceneRenderable::SceneRenderable(const glm::vec3& pos, const glm::quat& orient) :
        renderable_(nullptr),
        position_(pos),
        orientation_(orient),
        worldMatrix_(1.0f)
    {
        UpdatePositionOrientation(position_, orientation_);
    }

    void SceneRenderable::ResetScene(const glm::mat4& world)
    {
        worldMatrix_ = world;
        position_ = glm::vec3(world[3]);
        orientation_ = glm::quat_cast(world);
    }

    void SceneRenderable::UpdatePositionOrientation(const glm::vec3& pos, const glm::quat& orient)
    {
        position_ = pos;
        orientation_ = orient;
        glm::mat4 matOrient(glm::mat3_cast(orientation_));
        worldMatrix_ = glm::mat4(matOrient[0], matOrient[1], matOrient[2], glm::vec4(position_, 1));
    }

    void SceneRenderable::Draw(const ArcballCamera& camera) const
    {
        camera.SetView();
        renderable_->Draw(worldMatrix_);
    }

    void SceneRenderable::SaveScene(std::ostream& ostr, const PerspectiveCamera& camera) const
    {
        auto view = camera.GetViewMatrix();
        auto proj = camera.GetProjMatrix();

        serializeHelper::write(ostr, worldMatrix_);
        serializeHelper::write(ostr, view);
        serializeHelper::write(ostr, proj);
    }

    void SceneRenderable::LoadScene(std::istream& istr, PerspectiveCamera& camera)
    {
        glm::mat4 view, proj;
        serializeHelper::read(istr, worldMatrix_);
        serializeHelper::read(istr, view);
        serializeHelper::read(istr, proj);

        camera.ResetCamera(proj, view);
        ResetScene(worldMatrix_);
    }
}
