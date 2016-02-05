/**
 * @file   Configuration.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.18
 *
 * @brief  Implementation of the configuration class for windows systems.
 */

#include "Configuration.h"

namespace cgu {

    Configuration::Configuration() :
        fullscreen(false),
        backbufferBits(32),
        windowLeft(0),
        windowTop(0),
        windowWidth(800),
        windowHeight(600),
        useSRGB(false),
        pauseOnKillFocus(false),
        resourceBase("resources"),
        evalDirectory("evaluation"),
        useCUDA(true),
        cudaDevice(-1),
        sceneFile(""),
        fixCamera(false)
    {
    }

    Configuration::~Configuration() = default;
}
