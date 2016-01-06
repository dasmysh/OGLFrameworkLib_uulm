/**
 * @file   Configuration.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.18
 * @ingroup win
 *
 * @brief  Definition of the configuration class for windows systems.
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <boost/archive/xml_oarchive.hpp>

namespace cgu {

    /**
     * @brief  Configuration class used on windows systems for serializing the configuration.
     * @ingroup win
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2013.12.18
     */
    class Configuration
    {
        /** Deleted copy constructor. */
        Configuration(const Configuration&) = delete;

    public:
        Configuration();
        ~Configuration();

        /** Holds whether the main window is fullscreen. */
        bool fullscreen;
        /** Holds the bit depth of the back-buffer. */
        int backbufferBits;
        /** Holds the windows left position. */
        int windowLeft;
        /** Holds the windows top position. */
        int windowTop;
        /** Holds the windows width. */
        int windowWidth;
        /** Holds the windows height. */
        int windowHeight;
        /** Holds whether the back buffer should use sRGB. */
        bool useSRGB;
        /** Holds whether the application should pause on focus loss. */
        bool pauseOnKillFocus;
        /** Holds the resource base directory. */
        std::string resourceBase;
        /** Holds the resource base directory. */
        std::vector<std::string> resourceDirs;
        /** Holds whether to use CUDA in the application or not. */
        bool useCUDA;
        /** Holds the used CUDA device if CUDA is used. */
        int cudaDevice;

    private:
        /** Needed for serialization */
        friend class boost::serialization::access;

        /**
         * Saving method for boost serialization.
         * @param ar the archive to serialize to.
         * @param version the archives version.
         */
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const
        {
            ar & BOOST_SERIALIZATION_NVP(fullscreen);
            ar & BOOST_SERIALIZATION_NVP(backbufferBits);
            ar & BOOST_SERIALIZATION_NVP(windowLeft);
            ar & BOOST_SERIALIZATION_NVP(windowTop);
            ar & BOOST_SERIALIZATION_NVP(windowWidth);
            ar & BOOST_SERIALIZATION_NVP(windowHeight);
            if (version >= 4) {
                ar & BOOST_SERIALIZATION_NVP(useSRGB);
            }
            if (version >= 3) {
                ar & BOOST_SERIALIZATION_NVP(pauseOnKillFocus);
            }
            if (version >= 1) {
                ar & BOOST_SERIALIZATION_NVP(resourceBase);
            }
            if (version >= 5) {
                ar & BOOST_SERIALIZATION_NVP(resourceDirs);
            }
            if (version >= 2) {
                ar & BOOST_SERIALIZATION_NVP(useCUDA);
                ar & BOOST_SERIALIZATION_NVP(cudaDevice);
            }
        }

        /**
         * Saving method for boost serialization.
         * @param ar the archive to serialize to.
         * @param version the archives version.
         */
        template<class Archive>
        void load(Archive & ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(fullscreen);
            ar & BOOST_SERIALIZATION_NVP(backbufferBits);
            ar & BOOST_SERIALIZATION_NVP(windowLeft);
            ar & BOOST_SERIALIZATION_NVP(windowTop);
            ar & BOOST_SERIALIZATION_NVP(windowWidth);
            ar & BOOST_SERIALIZATION_NVP(windowHeight);
            if (version >= 4) {
                ar & BOOST_SERIALIZATION_NVP(useSRGB);
            }
            if (version >= 3) {
                ar & BOOST_SERIALIZATION_NVP(pauseOnKillFocus);
            }
            if (version >= 1) {
                ar & BOOST_SERIALIZATION_NVP(resourceBase);
            }
            if (version >= 5) {
                ar & BOOST_SERIALIZATION_NVP(resourceDirs);
            }
            if (version >= 2) {
                ar & BOOST_SERIALIZATION_NVP(useCUDA);
                ar & BOOST_SERIALIZATION_NVP(cudaDevice);
            }
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
}

BOOST_CLASS_VERSION(cgu::Configuration, 5)

#endif /* CONFIGURATION_H */
