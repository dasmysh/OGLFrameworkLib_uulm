/**
 * @file   AnimationManagerSerializationHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Implementation of the serialization helper for the animation manager.
 */

#include "AnimationManagerSerializationHelper.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
// ReSharper disable CppUnusedIncludeDirective
#include <boost/serialization/vector.hpp>
#include "gfx/WaypointAnimation.h"
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {

    void AnimationManagerSerializationHelper::LoadAnimation(std::istream& file, WaypointAnimation& animation)
    {
        boost::archive::xml_iarchive ia(file);
        ia >> BOOST_SERIALIZATION_NVP(animation);
    }

    void AnimationManagerSerializationHelper::SaveAnimation(std::ostream& file, const WaypointAnimation& animation)
    {
        boost::archive::xml_oarchive oa(file);
        oa << BOOST_SERIALIZATION_NVP(animation);
    }

    void AnimationManagerSerializationHelper::LoadAnimations(std::istream& file, std::vector<WaypointAnimation>& animations, std::vector<std::string>& animationNames)
    {
        boost::archive::xml_iarchive ia(file);
        ia >> BOOST_SERIALIZATION_NVP(animations);
        ia >> BOOST_SERIALIZATION_NVP(animationNames);
    }

    void AnimationManagerSerializationHelper::SaveAnimations(std::ostream& file, const std::vector<WaypointAnimation>& animations, const std::vector<std::string>& animationNames)
    {
        boost::archive::xml_oarchive oa(file);
        oa << BOOST_SERIALIZATION_NVP(animations);
        oa << BOOST_SERIALIZATION_NVP(animationNames);
    }

    AnimationManagerSerializationHelper::AnimationManagerSerializationHelper()
    {
    }


    AnimationManagerSerializationHelper::~AnimationManagerSerializationHelper() = default;
}
