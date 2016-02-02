/**
 * @file   GLTexture3D.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.14
 *
 * @brief  Contains the implementation of GLTexture3D.
 */

#define GLM_SWIZZLE
#include "Volume.h"
#include "app/ApplicationBase.h"
#include <codecvt>
#include <fstream>
#include "gfx/glrenderer/GLTexture.h"
#include <ios>
#include <boost/filesystem.hpp>
#include "app/Configuration.h"
#include <limits>
#include <boost/lexical_cast.hpp>

/*#undef min
#undef max*/

namespace cgu {

    template<typename OT, typename OET, typename I>
    static std::vector<OT> readModifyData(const std::vector<char>& rawData, unsigned int size, std::function<OET(const I&)> modify)
    {
        auto elementsToRead = size / static_cast<unsigned int>(sizeof(I));
        auto ptr = reinterpret_cast<const I*>(rawData.data());
        std::vector<OT> data(elementsToRead * sizeof(OET) / sizeof(OT));
        for (unsigned int i = 0; i < elementsToRead; ++i) {
            reinterpret_cast<OET*>(data.data())[i] = modify(ptr[i]);
        }
        return std::move(data);
    }

    /**
     * Constructor.
     * @param texFilename the textures file name
     * @param app the application object
     */
    Volume::Volume(const std::string& texFilename, ApplicationBase* app) :
        Resource{ texFilename, app },
        volumeSize(0),
        cellSize(1.0f),
        scaleValue(1),
        dataDim(1),
        texDesc(4, GL_R8, GL_RED, GL_UNSIGNED_BYTE)
    {
        LoadDatFile();
    }

    /** Copy constructor. */
    Volume::Volume(const Volume& rhs) : Volume(rhs.getId(), rhs.application)
    {
    }

    /** Copy assignment operator. */
    Volume& Volume::operator=(const Volume& rhs)
    {
        if (this != &rhs) {
            auto tmp(rhs);
            std::swap(*this, tmp);
        }
        return *this;
    }

    /** Move constructor. */
    Volume::Volume(Volume&& rhs) :
        Resource(std::move(rhs)),
        volumeSize(std::move(rhs.volumeSize)),
        cellSize(std::move(rhs.cellSize)),
        rawFileName(std::move(rhs.rawFileName)),
        scaleValue(std::move(rhs.scaleValue)),
        dataDim(std::move(rhs.dataDim)),
        texDesc(std::move(rhs.texDesc))
    {
        
    }
    Volume& Volume::operator=(Volume&& rhs)
    {
        this->~Volume();
        volumeSize = std::move(rhs.volumeSize);
        cellSize = std::move(rhs.cellSize);
        rawFileName = std::move(rhs.rawFileName);
        scaleValue = std::move(rhs.scaleValue);
        dataDim = std::move(rhs.dataDim);
        texDesc = std::move(rhs.texDesc);
        return *this;
    }

    /** Destructor. */
    Volume::~Volume() = default;

    /**
     *  Loads the dat file.
     */
    void Volume::LoadDatFile()
    {
        auto filename = FindResourceLocation(GetParameters()[0]);
        auto forceBits = GetNamedParameterValue<unsigned int>("forceBits", 0);

        boost::filesystem::path datFile{ filename };
        auto path = datFile.parent_path().string() + "/";
        auto ending = datFile.extension().string();

        if (ending != ".dat" && ending != ".DAT") {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Cannot load '" << converter.from_bytes(ending) << "' Only .dat files are supported.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(getId())
                << errdesc_info("Cannot load file, file type not supported.");
        }

        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Cannot open file '" << converter.from_bytes(filename) << "'.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(getId())
                << errdesc_info("Cannot open file.");
        }

        std::string str, raw_file, format_str, obj_model;

        while (ifs >> str && ifs.good()) {
            if (str == "ObjectFileName:")
                ifs >> raw_file;
            else if (str == "Resolution:") {
                ifs >> volumeSize.x; ifs >> volumeSize.y; ifs >> volumeSize.z;
            } else if (str == "SliceThickness:") {
                ifs >> cellSize.x; ifs >> cellSize.y; ifs >> cellSize.z;
            } else if (str == "Format:")
                ifs >> format_str;
            else if (str == "ObjectModel:")
                ifs >> obj_model;
        }
        ifs.close();

        if (raw_file == "" || volumeSize == glm::uvec3(0) || format_str == "") {
            LOG(ERROR) << "Could find all required fields in dat file.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(getId())
                << errdesc_info("Cannot find all required fields in dat file.");
        }

        unsigned int componentSize;
        if (format_str == "UCHAR") {
            texDesc.type = GL_UNSIGNED_BYTE;
            componentSize = 1;
        } else if (format_str == "USHORT") {
            texDesc.type = GL_UNSIGNED_SHORT;
            componentSize = 2;
        } else if (format_str == "USHORT_12") {
            texDesc.type = GL_UNSIGNED_SHORT;
            componentSize = 2;
        } else if (format_str == "UINT") {
            texDesc.type = GL_UNSIGNED_INT;
            componentSize = 4;
        } else if (format_str == "FLOAT") {
            texDesc.type = GL_FLOAT;
            componentSize = 4;
        } else {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Format '" << converter.from_bytes(format_str) << "' is not supported.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(getId())
                << errdesc_info("Format not supported.");
        }

        if (obj_model == "I") {
            dataDim = 1;
            texDesc.format = GL_RED;
        } else if (obj_model == "RG" || obj_model == "XY") {
            dataDim = 2;
            texDesc.format = GL_RG;
        } else if (obj_model == "RGB" || obj_model == "XYZ") {
            dataDim = 3;
            texDesc.format = GL_RGB;
        } else if (obj_model == "RGBA" || obj_model == "XYZW") {
            dataDim = 4;
            texDesc.format = GL_RGBA;
        } else {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "ObjectModel '" << converter.from_bytes(obj_model) << "' is not supported.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(getId())
                << errdesc_info("ObjectModel not supported.");
        }

        if (forceBits == 0) {
            texDesc.bytesPP = dataDim * componentSize;
            if (texDesc.type == GL_UNSIGNED_BYTE && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R8;
            else if (texDesc.type == GL_UNSIGNED_BYTE && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG8;
            else if (texDesc.type == GL_UNSIGNED_BYTE && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB8;
            else if (texDesc.type == GL_UNSIGNED_BYTE && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA8;
            else if (texDesc.type == GL_UNSIGNED_SHORT && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R16F;
            else if (texDesc.type == GL_UNSIGNED_SHORT && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG16F;
            else if (texDesc.type == GL_UNSIGNED_SHORT && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB16F;
            else if (texDesc.type == GL_UNSIGNED_SHORT && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA16F;
            else if (componentSize == 4 && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R32F;
            else if (componentSize == 4 && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG32F;
            else if (componentSize == 4 && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB32F;
            else if (componentSize == 4 && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA32F;
        } else {
            texDesc.bytesPP = dataDim * (forceBits / 8);
            if (forceBits == 8 && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R8;
            else if (forceBits == 8 && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG8;
            else if (forceBits == 8 && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB8;
            else if (forceBits == 8 && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA8;
            else if (forceBits == 16 && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R16F;
            else if (forceBits == 16 && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG16F;
            else if (forceBits == 16 && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB16F;
            else if (forceBits == 16 && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA16F;
            else if (forceBits == 32 && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R32F;
            else if (forceBits == 32 && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG32F;
            else if (forceBits == 32 && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB32F;
            else if (forceBits == 32 && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA32F;
        }

        scaleValue = (format_str == "USHORT_12") ? 16 : 1;
        rawFileName = path + "/" + raw_file;
    }

    void Volume::LoadRawDataFromFile(unsigned& data_size, std::vector<char>& rawData) const
    {
        std::ifstream ifsRaw(rawFileName, std::ios::in | std::ios::binary);
        if (!ifsRaw || !ifsRaw.is_open()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Could not open file '" << converter.from_bytes(rawFileName) << "'.";
            throw std::runtime_error("Could not open file '" + rawFileName + "'.");
        }

        ifsRaw.seekg(0, std::ios::end);
        data_size = static_cast<unsigned int>(ifsRaw.tellg());
        ifsRaw.seekg(0, std::ios::beg);

        rawData = std::vector<char>(data_size, 9);
        ifsRaw.read(rawData.data(), data_size);
        ifsRaw.close();
    }

    /**
         *  Loads the content of the volume to a 3D texture.
         *  @param mipLevels the number of MipMap levels the texture should have.
         *  @return the loaded texture.
         */
    std::unique_ptr<GLTexture> Volume::Load3DTexture(unsigned int mipLevels) const
    {
        unsigned data_size;
        std::vector<char> rawData;
        LoadRawDataFromFile(data_size, rawData);

        // std::vector<int8_t> data;
        std::vector<float> data;
        if (texDesc.type == GL_UNSIGNED_BYTE) {
            // data = readModifyData<int8_t, uint8_t, uint8_t>(rawData, data_size, [](const uint8_t& val){ return val; });
            data = readModifyData<float, float, uint8_t>(rawData, data_size, [](const uint8_t& val){ return static_cast<float>(val) / static_cast<float>(std::numeric_limits<uint8_t>::max()); });
        } else if (texDesc.type == GL_UNSIGNED_SHORT) {
            auto l_scaleValue = scaleValue;
            // data = readModifyData<int8_t, uint16_t, uint16_t>(rawData, data_size, [l_scaleValue](const uint16_t& val){ return val * l_scaleValue; });
            data = readModifyData<float, float, uint16_t>(rawData, data_size, [l_scaleValue](const uint16_t& val){ return static_cast<float>(val * l_scaleValue) / static_cast<float>(std::numeric_limits<uint16_t>::max()); });
        } else if (texDesc.type == GL_UNSIGNED_INT) {
            // data = readModifyData<int8_t, uint32_t, uint32_t>(rawData, data_size, [](const uint32_t& val){ return val; });
            data = readModifyData<float, float, uint32_t>(rawData, data_size, [](const uint32_t& val){ return static_cast<float>(val) / static_cast<float>(std::numeric_limits<uint32_t>::max()); });
        } else if (texDesc.type == GL_FLOAT) {
            // data = readModifyData<int8_t, float, float>(rawData, data_size, [](const float& val){ return val; });
            data = readModifyData<float, float, float>(rawData, data_size, [](const float& val){ return val; });
        }

        auto tempDesc = texDesc;
        tempDesc.type = GL_FLOAT;
        auto volTex = std::make_unique<GLTexture>(volumeSize.x, volumeSize.y, volumeSize.z, mipLevels, tempDesc, data.data());
        return std::move(volTex);
    }

    std::shared_ptr<Volume> Volume::GetSpeedVolume() const
    {
        assert(texDesc.format == GL_RGBA);

        boost::filesystem::path volumeRelativeFilename(GetParameters()[0]);
        boost::filesystem::path volumeFilename(FindResourceLocation(GetParameters()[0]));

        auto newStrippedFilename = volumeFilename.filename().stem().string() + "_speed";

        auto newDatFilename = volumeFilename.parent_path().string() + "/" + newStrippedFilename + ".dat";
        auto newRelativeDatFilename = volumeRelativeFilename.parent_path().string() + "/" + newStrippedFilename + ".dat";
        auto newRawFilename = volumeFilename.parent_path().string() + "/" + newStrippedFilename + ".raw";

        if (!boost::filesystem::exists(newDatFilename)) {
            std::ofstream datOut(newDatFilename, std::ofstream::out | std::ofstream::trunc);
            if (!datOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newDatFilename) << "'.";
                throw std::runtime_error("Could not open file '" + newDatFilename + "'.");
            }

            std::string newFormat = "UCHAR";
            if (texDesc.type == GL_UNSIGNED_SHORT) newFormat = "USHORT";
            else if (texDesc.type == GL_UNSIGNED_INT) newFormat = "UINT";

            datOut << "ObjectFileName:\t" << newRawFilename << ".raw" << std::endl;
            datOut << "Resolution:\t" << volumeSize.x << " " << volumeSize.y << " " << volumeSize.z << std::endl;
            datOut << "SliceThickness:\t" << cellSize.x << " " << cellSize.y << " " << cellSize.z << std::endl;
            datOut << "Format:\t" << newFormat << std::endl;
            datOut << "ObjectModel:\tI" << std::endl;

            datOut.close();

            std::fstream rawOut(newRawFilename, std::ios_base::in | std::ios_base::out | std::ios_base::binary
                | std::ios_base::trunc);
            if (!rawOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newRawFilename) << "'.";
                throw std::runtime_error("Could not open file '" + newRawFilename + "'.");
            }

            unsigned data_size;
            std::vector<char> rawData;
            LoadRawDataFromFile(data_size, rawData);

            std::vector<int8_t> data;
            auto volumeNumBytes = volumeSize.x * volumeSize.y * volumeSize.z * texDesc.bytesPP;

            int size = std::min(data_size, volumeNumBytes);
            if (texDesc.type == GL_UNSIGNED_BYTE) {
                data = readModifyData<int8_t, uint8_t, glm::u8vec4>(rawData, size, [](const glm::u8vec4& val)
                {
                    auto sval = glm::vec3(val.xyz()) / glm::vec3(std::numeric_limits<uint8_t>::max());
                    return static_cast<uint8_t>(glm::length((sval - glm::vec3(0.5f)) * 2.0f) * static_cast<float>(std::numeric_limits<uint8_t>::max()));
                });
            } else if (texDesc.type == GL_UNSIGNED_SHORT) {
                auto l_scaleValue = scaleValue;
                data = readModifyData<int8_t, uint16_t, glm::u16vec4>(rawData, size, [l_scaleValue](const glm::u16vec4& val)
                {
                    auto sval = glm::vec3(val.xyz() * glm::u16vec3(l_scaleValue)) / glm::vec3(std::numeric_limits<uint16_t>::max());
                    return static_cast<uint16_t>(glm::length((sval - glm::vec3(0.5f)) * 2.0f) * static_cast<float>(std::numeric_limits<uint16_t>::max()));
                });
            } else if (texDesc.type == GL_UNSIGNED_INT) {
                data = readModifyData<int8_t, uint32_t, glm::u32vec4>(rawData, size, [](const glm::u32vec4& val)
                {
                    auto sval = glm::vec3(val.xyz()) / glm::vec3(static_cast<float>(std::numeric_limits<uint32_t>::max()));
                    return static_cast<uint8_t>(glm::length((sval - glm::vec3(0.5f)) * 2.0f) * static_cast<float>(std::numeric_limits<uint32_t>::max()));
                });
            }

            auto dataPtr = reinterpret_cast<char*>(data.data());
            rawOut.seekp(0, std::ios::beg);
            rawOut.write(dataPtr, size);
            rawOut.close();
        }

        std::string newFileParameters;
        for (unsigned int i = 1; i < GetParameters().size(); ++i) {
            newFileParameters += "," + GetParameter(i);
        }
        return application->GetVolumeManager()->GetResource(newRelativeDatFilename + newFileParameters);
    }
}
