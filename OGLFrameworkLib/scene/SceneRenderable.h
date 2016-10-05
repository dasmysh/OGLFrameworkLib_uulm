/**
 * @file   SceneRenderable.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.28
 *
 * @brief  Definition of a renderable object inside a scene.
 */

#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ostream>
#include <istream>

namespace cgu {

    class ArcballCamera;
    class PerspectiveCamera;
    class GPUProgram;
    class MeshRenderable;

    class SceneRenderable
    {
    public:
        // TODO: refactor this class and remove renderable. [10/5/2016 Sebastian Maisch]
        SceneRenderable(MeshRenderable* renderable, const glm::vec3& pos, const glm::quat& orient);
        virtual ~SceneRenderable();

        void UpdatePositionOrientation(const glm::vec3& pos, const glm::quat& orient);
        void Draw(const ArcballCamera& camera) const;
        virtual void SaveScene(std::ostream& ostr, const PerspectiveCamera& camera) const;
        virtual void LoadScene(std::istream& istr, PerspectiveCamera& camera);

        const glm::quat& GetOrientation() const { return orientation_; }
        const glm::mat4& GetWorldMatrix() const { return worldMatrix_; }
        const glm::vec3& GetPosition() const { return position_; }

    protected:
        /** Constructor used for derivations that create their own renderables. */
        SceneRenderable(const glm::vec3& pos, const glm::quat& orient);
        /** Reset the scene orientation and position. */
        void ResetScene(const glm::mat4& world);

    private:
        /** Holds the renderable used for display. */
        MeshRenderable* renderable_;
        /** Holds the objects position in space. */
        glm::vec3 position_;
        /** Holds the objects orientation. */
        glm::quat orientation_;
        /** Holds the objects world matrix. */
        glm::mat4 worldMatrix_;
    };
}

#endif /* SCENEOBJECT_H */
