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
#include <fstream>

namespace cgu {

    namespace serializeHelper {
        template<class T> void write(std::ofstream& ofs, const T& value) { ofs.write(reinterpret_cast<const char*>(&value), sizeof(T)); }
        template<> inline void write<std::string>(std::ofstream& ofs, const std::string& value) { write(ofs, static_cast<uint32_t>(value.size())); ofs.write(value.data(), value.size()); }

        template<class T> void writeV(std::ofstream& ofs, const std::vector<T>& value) { write(ofs, static_cast<uint32_t>(value.size())); ofs.write(reinterpret_cast<const char*>(value.data()), value.size() * sizeof(T)); }
        template<> inline void writeV<std::string>(std::ofstream& ofs, const std::vector<std::string>& value) { write(ofs, static_cast<uint32_t>(value.size())); for (const auto& str : value) write(ofs, str); }

        template<class T> void writeVV(std::ofstream& ofs, const std::vector<std::vector<T>>& value) { write(ofs, static_cast<uint32_t>(value.size())); for (const auto& str : value) writeV(ofs, str); }


        template<class T> void read(std::ifstream& ifs, T& value) { ifs.read(reinterpret_cast<char*>(&value), sizeof(T)); }
        template<> inline void read<std::string>(std::ifstream& ifs, std::string& value) {
            decltype(value.size()) strLength; ifs.read(reinterpret_cast<char*>(&strLength), sizeof(strLength));
            value.resize(strLength); ifs.read(const_cast<char*>(value.c_str()), strLength);
        }

        template<class T> void readV(std::ifstream& ifs, std::vector<T>& value) {
            decltype(value.size()) vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
            value.resize(vecLength); ifs.read(reinterpret_cast<char*>(value.data()), vecLength * sizeof(T));
        }
        template<> inline void readV<std::string>(std::ifstream& ifs, std::vector<std::string>& value) {
            decltype(value.size()) vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
            value.resize(vecLength); for (auto& str : value) read(ifs, str);
        }

        template<class T> void readVV(std::ifstream& ifs, std::vector<std::vector<T>>& value) {
            decltype(value.size()) vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
            value.resize(vecLength); for (auto& str : value) readV(ifs, str);
        }
    }
}


#endif // SERIALIZATIONHELPER_H



