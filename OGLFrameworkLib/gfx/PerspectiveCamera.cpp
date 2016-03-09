/**
 * @file   PerspectiveCamera.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.03.02
 *
 * @brief  Implementation of a perspective camera.
 */

#define GLM_SWIZZLE

#include "PerspectiveCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include "glrenderer/GLUniformBuffer.h"

namespace cgu {

    /**
     *  Constructor.
     *  @param theFov the cameras field of view.
     *  @param theScreenSize the cameras screen size.
     *  @param theNearZ distance to the near clipping plane.
     *  @param theFarZ distance to the far clipping plane.
     *  @param theCamPos the cameras position.
     *  @param uniformBindingPoints uniform buffer binding points for the camera used for shadow map rendering.
     */
    PerspectiveCamera::PerspectiveCamera(float theFovY, const glm::uvec2& theScreenSize, float theNearZ, float theFarZ,
        const glm::vec3& theCamPos, ShaderBufferBindingPoints* uniformBindingPoints) :
        fovY_(theFovY),
        aspectRatio_(static_cast<float>(theScreenSize.x) / static_cast<float>(theScreenSize.y)),
        screenSize_(theScreenSize),
        nearZ_(theNearZ),
        farZ_(theFarZ),
        camPos_(theCamPos),
        camOrient_(1.0f, 0.0f, 0.0f, 0.0f),
        camUp_(0.0f, 1.0f, 0.0f),
        perspectiveUBO_(uniformBindingPoints == nullptr ? nullptr : std::make_unique<GLUniformBuffer>(perspectiveProjectionUBBName,
            static_cast<unsigned int>(sizeof(PerspectiveParams)), uniformBindingPoints))
    {
        Resize(screenSize_);
    }

    PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& rhs) :
        fovY_(rhs.fovY_),
        aspectRatio_(rhs.aspectRatio_),
        screenSize_(rhs.screenSize_),
        nearZ_(rhs.nearZ_),
        farZ_(rhs.farZ_),
        perspective_(rhs.perspective_),
        camPos_(rhs.camPos_),
        camOrient_(rhs.camOrient_),
        camUp_(rhs.camUp_),
        view_(rhs.view_),
        perspectiveUBO_(new GLUniformBuffer(*rhs.perspectiveUBO_))
    {
    }

    PerspectiveCamera& PerspectiveCamera::operator=(const PerspectiveCamera& rhs)
    {
        if (this != &rhs) {
            PerspectiveCamera tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    PerspectiveCamera::PerspectiveCamera(PerspectiveCamera&& rhs) :
        fovY_(std::move(rhs.fovY_)),
        aspectRatio_(std::move(rhs.aspectRatio_)),
        screenSize_(std::move(rhs.screenSize_)),
        nearZ_(std::move(rhs.nearZ_)),
        farZ_(std::move(rhs.farZ_)),
        perspective_(std::move(rhs.perspective_)),
        camPos_(std::move(rhs.camPos_)),
        camOrient_(std::move(rhs.camOrient_)),
        camUp_(std::move(rhs.camUp_)),
        view_(std::move(rhs.view_)),
        perspectiveUBO_(std::move(rhs.perspectiveUBO_))
    {
    }

    PerspectiveCamera& PerspectiveCamera::operator=(PerspectiveCamera&& rhs)
    {
        if (this != &rhs) {
            this->~PerspectiveCamera();
            fovY_ = rhs.fovY_;
            aspectRatio_ = rhs.aspectRatio_;
            screenSize_ = rhs.screenSize_;
            nearZ_ = rhs.nearZ_;
            farZ_ = rhs.farZ_;
            perspective_ = rhs.perspective_;
            camPos_ = rhs.camPos_;
            camOrient_ = rhs.camOrient_;
            camUp_ = rhs.camUp_;
            view_ = rhs.view_;
            perspectiveUBO_ = std::move(rhs.perspectiveUBO_);
        }
        return *this;
    }

    PerspectiveCamera::~PerspectiveCamera() = default;

    void PerspectiveCamera::ResetCamera(const glm::mat4& proj, const glm::mat4& view)
    {
        perspective_ = proj;
        nearZ_ = proj[3][2] / (proj[2][2] - 1);
        farZ_ = proj[3][2] / (proj[2][2] + 1);
        auto t = proj[3][2] / ((proj[2][2] - 1) * proj[1][1]);
        auto r = proj[3][2] / ((proj[2][2] - 1) * proj[0][0]);

        fovY_ = 2.f * glm::atan(t / nearZ_);
        aspectRatio_ = r / t;

        view_ = view;

        auto viewInv = glm::inverse(view);
        camOrient_ = glm::quat_cast(viewInv);
        camPos_ = glm::vec3(viewInv[3]);
        camUp_ = glm::vec3(viewInv[1]);
    }

    void PerspectiveCamera::Resize(const glm::uvec2& screenSize)
    {
        screenSize_ = screenSize;
        aspectRatio_ = static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y);
        perspective_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearZ_, farZ_);
        view_ = glm::lookAt(camPos_, glm::vec3(0.0f), camUp_);
    }

    void PerspectiveCamera::SetView() const
    {
        PerspectiveParams params{ perspective_ * view_, camPos_ };

        if (perspectiveUBO_) {
            perspectiveUBO_->UploadData(0, sizeof(PerspectiveParams), &params);
            perspectiveUBO_->BindBuffer();
        }
    }

    void PerspectiveCamera::SetViewShadowMap() const
    {
        /** see http://www.mvps.org/directx/articles/linear_z/linearz.htm */
        auto projectionLinear = perspective_;
        // auto Q = perspective[2][2];
        /*auto N = -perspective[3][2] / ( -perspective[2][2] + 1.0f);
        auto F = -perspective[3][2] / ( -perspective[2][2] - 1.0f);
        projectionLinear[2][2] /= F;
        projectionLinear[3][2] /= F;*/
        PerspectiveParams params{ perspective_ * view_, camPos_ };

        if (perspectiveUBO_) {
            perspectiveUBO_->UploadData(0, sizeof(PerspectiveParams), &params);
            perspectiveUBO_->BindBuffer();
        }
    }

    void PerspectiveCamera::RotateOrigin(const glm::quat& camOrientStep)
    {
        glm::mat3 matOrientStep{ glm::mat3_cast(camOrientStep) };
        camOrient_ = camOrientStep * GetOrientation();
        glm::mat3 matOrient{ glm::mat3_cast(camOrient_) };

        camUp_ = matOrient[1];
        camPos_ = matOrientStep * camPos_;

        view_ = glm::lookAt(camPos_, glm::vec3(0.0f), camUp_);
    }

    void PerspectiveCamera::MoveCamera(const glm::vec3& translation)
    {
        camPos_ += translation;
        view_ = glm::lookAt(camPos_, glm::vec3(0.0f), camUp_);
    }

    cguMath::Frustum<float> PerspectiveCamera::GetViewFrustum(const glm::mat4& modelM) const
    {
        auto mvp = perspective_ * view_ * modelM;
        return std::move(CalcViewFrustum(mvp));
    }

    float PerspectiveCamera::GetSignedDistance2ToUnitAABB(const glm::mat4& world) const
    {
        auto localCamPos = glm::vec3(glm::inverse(world) * glm::vec4(camPos_, 1.0f));
        auto clampedCamPos = glm::vec3(world * glm::vec4(glm::clamp(localCamPos, glm::vec3(0.0f), glm::vec3(1.0f)), 1.0f));
        return glm::dot(clampedCamPos - camPos_, clampedCamPos - camPos_);
    }

    glm::vec2 PerspectiveCamera::CalculatePixelFootprintToUnitAABB(const glm::mat4& world) const
    {
        auto mvp = perspective_ * view_ * world;
        glm::vec4 pmin, pmax;
        pmin = pmax = mvp[3];
        for (unsigned int i = 0; i < 4; ++i) {
            for (unsigned int j = 0; j < 4; ++j) {
                if (mvp[j][i] < 0.0f) pmin[i] += mvp[j][i];
                else pmax[i] += mvp[j][i];
            }
        }
        pmin /= pmin.w;
        pmax /= pmax.w;

        cguMath::AABB2<float> ssAABB{ { { glm::vec2(pmin), glm::vec2(pmax) } } };
        ssAABB.minmax[0] = (ssAABB.minmax[0] + glm::vec2(1.0f)) * 0.5f * glm::vec2(screenSize_);
        ssAABB.minmax[1] = (ssAABB.minmax[1] + glm::vec2(1.0f)) * 0.5f * glm::vec2(screenSize_);

        return std::move(ssAABB.minmax[1] - ssAABB.minmax[0]);
    }

    float PerspectiveCamera::GetSignedDistanceToUnitAABB2(const glm::mat4& world) const
    {
        auto localCamPos = glm::vec3(glm::inverse(world) * glm::vec4(camPos_, 1.0f));
        auto clampedCamPos = glm::vec3(world * glm::vec4(glm::clamp(localCamPos, glm::vec3(0.0f), glm::vec3(1.0f)), 1.0f));
        return glm::dot(clampedCamPos - camPos_, clampedCamPos - camPos_);
    }

    void PerspectiveCamera::SetFOV(float fov)
    {
        fovY_ = fov;
        perspective_ = glm::perspective(glm::radians(fovY_), aspectRatio_, nearZ_, farZ_);
    }

    cguMath::Frustum<float> PerspectiveCamera::CalcViewFrustum(const glm::mat4& mvp) const
    {
        cguMath::Frustum<float> f;
        f.left() = glm::row(mvp, 3) + glm::row(mvp, 0);
        f.rght() = glm::row(mvp, 3) - glm::row(mvp, 0);
        f.bttm() = glm::row(mvp, 3) + glm::row(mvp, 1);
        f.topp() = glm::row(mvp, 3) - glm::row(mvp, 1);
        f.nrpl() = glm::row(mvp, 3) + glm::row(mvp, 2);
        f.farp() = glm::row(mvp, 3) - glm::row(mvp, 2);

        /*f.left() = glm::vec4{ m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0], m[3][3] + m[3][0] };
        f.rght() = glm::vec4{ m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0], m[3][3] - m[3][0] };
        f.bttm() = glm::vec4{ m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1], m[3][3] + m[3][1] };
        f.topp() = glm::vec4{ m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1], m[3][3] - m[3][1] };
        f.nrpl() = glm::vec4{ m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2], m[3][3] + m[3][2] };
        f.farp() = glm::vec4{ m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2], m[3][3] - m[3][2] };*/
        f.left() /= glm::length(glm::vec3(f.left()));
        f.rght() /= glm::length(glm::vec3(f.rght()));
        f.bttm() /= glm::length(glm::vec3(f.bttm()));
        f.topp() /= glm::length(glm::vec3(f.topp()));
        f.nrpl() /= glm::length(glm::vec3(f.nrpl()));
        f.farp() /= glm::length(glm::vec3(f.farp()));
        return std::move(f);
    }
}
