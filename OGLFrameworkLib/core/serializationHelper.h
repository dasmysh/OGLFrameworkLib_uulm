/**
 * @file   serializationHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.17
 *
 * @brief  Definition of helper functions for serializing objects.
 */

#ifndef SERIALIZATIONHELPER_H
#define SERIALIZATIONHELPER_H

#include "main.h"
#include <istream>
#include <ostream>
#include <boost/serialization/nvp.hpp>

namespace cgu {

    namespace serializeHelper {
        inline unsigned int tag(char a, char b, char c, char d) {
            return (static_cast<unsigned int>(a) << 24 | static_cast<unsigned int>(b) << 16 | static_cast<unsigned int>(c) << 8 | static_cast<unsigned int>(d));
        }

        template<class T> void write(std::ostream& ofs, const T& value) { ofs.write(reinterpret_cast<const char*>(&value), sizeof(T)); }
        template<> inline void write<std::string>(std::ostream& ofs, const std::string& value) { write(ofs, static_cast<uint64_t>(value.size())); ofs.write(value.data(), value.size()); }

        template<class T> void writeV(std::ostream& ofs, const std::vector<T>& value)
        {
            write(ofs, static_cast<uint64_t>(value.size()));
            ofs.write(reinterpret_cast<const char*>(value.data()), value.size() * sizeof(T));
        }
        template<> inline void writeV<std::string>(std::ostream& ofs, const std::vector<std::string>& value) { write(ofs, static_cast<uint64_t>(value.size())); for (const auto& str : value) write(ofs, str); }

        template<class T> void writeVV(std::ostream& ofs, const std::vector<std::vector<T>>& value) { write(ofs, static_cast<uint64_t>(value.size())); for (const auto& str : value) writeV(ofs, str); }


        template<class T> void read(std::istream& ifs, T& value) { ifs.read(reinterpret_cast<char*>(&value), sizeof(T)); }
        template<> inline void read<std::string>(std::istream& ifs, std::string& value) {
            uint64_t strLength; ifs.read(reinterpret_cast<char*>(&strLength), sizeof(strLength));
            value.resize(strLength); if (strLength != 0) ifs.read(const_cast<char*>(value.c_str()), strLength);
        }

        template<class T> void readV(std::istream& ifs, std::vector<T>& value) {
            uint64_t vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
            value.resize(vecLength); if (vecLength != 0) ifs.read(reinterpret_cast<char*>(value.data()), vecLength * sizeof(T));
        }
        template<> inline void readV<std::string>(std::istream& ifs, std::vector<std::string>& value) {
            uint64_t vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
            value.resize(vecLength); for (auto& str : value) read(ifs, str);
        }

        template<class T> void readVV(std::istream& ifs, std::vector<std::vector<T>>& value) {
            uint64_t vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
            value.resize(vecLength); for (auto& str : value) readV(ifs, str);
        }

        template<char T0, char T1, char T2, char T3, unsigned int V> struct VersionableSerializer
        {
            using VersionableSerializerType = VersionableSerializer<T0, T1, T2, T3, V>;
            static const unsigned int VERSION = V;

            static std::tuple<bool, unsigned int> checkHeader(std::istream& ifs)
            {
                const auto TAG = tag(T0, T1, T2, T3);
                unsigned int ftag, fversion;
                read(ifs, ftag);
                read(ifs, fversion);
                if (ftag == TAG) return std::make_tuple(fversion == V, fversion);
                else return std::make_tuple(false, 0);
            }

            static void writeHeader(std::ostream& ofs)
            {
                const auto TAG = tag(T0, T1, T2, T3);
                write(ofs, TAG);
                write(ofs, V);
            }
        };
    }
}

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive & ar, glm::vec2& g, const unsigned int)
        {
            ar & make_nvp(BOOST_PP_STRINGIZE(x), g.x);
            ar & make_nvp(BOOST_PP_STRINGIZE(y), g.y);
        }

        template<class Archive>
        void serialize(Archive & ar, glm::vec3& g, const unsigned int)
        {
            ar & make_nvp(BOOST_PP_STRINGIZE(x), g.x);
            ar & make_nvp(BOOST_PP_STRINGIZE(y), g.y);
            ar & make_nvp(BOOST_PP_STRINGIZE(z), g.z);
        }

        template<class Archive>
        void serialize(Archive & ar, glm::vec4& g, const unsigned int)
        {
            ar & make_nvp(BOOST_PP_STRINGIZE(x), g.x);
            ar & make_nvp(BOOST_PP_STRINGIZE(y), g.y);
            ar & make_nvp(BOOST_PP_STRINGIZE(z), g.z);
            ar & make_nvp(BOOST_PP_STRINGIZE(w), g.w);
        }

        template<class Archive>
        void serialize(Archive & ar, glm::quat& g, const unsigned int)
        {
            ar & make_nvp(BOOST_PP_STRINGIZE(x), g.x);
            ar & make_nvp(BOOST_PP_STRINGIZE(y), g.y);
            ar & make_nvp(BOOST_PP_STRINGIZE(z), g.z);
            ar & make_nvp(BOOST_PP_STRINGIZE(w), g.w);
        }
    }
}


#endif // SERIALIZATIONHELPER_H



