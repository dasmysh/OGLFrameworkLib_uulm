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
#include <boost/assign.hpp>
#include <ios>
#include <boost/filesystem.hpp>
#include "app/Configuration.h"
#include <limits>

#undef min
#undef max

namespace cgu {

    template<typename OT, typename OET, typename I>
    static std::vector<OT> readModifyData(const std::vector<char>& rawData, unsigned int size, std::function<OET(const I&)> modify)
    {
        auto elementsToRead = size / static_cast<unsigned int>(sizeof(I));
        auto ptr = reinterpret_cast<I*>(rawData.data());
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
    }

    /** Copy constructor. */
    Volume::Volume(const Volume& rhs) : Volume(rhs.id, rhs.application)
    {
        if (rhs.IsLoaded()) Volume::Load();
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

    void Volume::Load()
    {
        LoadDatFile();

        Resource::Load();
    }

    /**
     *  Loads the dat file.
     */
    void Volume::LoadDatFile()
    {
        auto filename = FindResourceLocation(GetParameters()[0]);
        boost::filesystem::path datFile{ filename };
        auto path = datFile.parent_path().string() + "/";
        auto ending = datFile.extension().string();

        if (ending != ".dat" && ending != ".DAT") {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Cannot load '" << converter.from_bytes(ending) << "' Only .dat files are supported.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(id)
                << errdesc_info("Cannot load file, file type not supported.");
        }

        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Cannot open file '" << converter.from_bytes(filename) << "'.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(id)
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
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(id)
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
        } else {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Format '" << converter.from_bytes(format_str) << "' is not supported.";
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(id)
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
            throw resource_loading_error() << ::boost::errinfo_file_name(datFile.filename().string()) << resid_info(id)
                << errdesc_info("ObjectModel not supported.");
        }

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
        else if (texDesc.type == GL_UNSIGNED_INT && texDesc.format == GL_RED)
            texDesc.internalFormat = GL_R32F;
        else if (texDesc.type == GL_UNSIGNED_INT && texDesc.format == GL_RG)
            texDesc.internalFormat = GL_RG32F;
        else if (texDesc.type == GL_UNSIGNED_INT && texDesc.format == GL_RGB)
            texDesc.internalFormat = GL_RGB32F;
        else if (texDesc.type == GL_UNSIGNED_INT && texDesc.format == GL_RGBA)
            texDesc.internalFormat = GL_RGBA32F;

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

        std::vector<int8_t> data;
        auto volumeNumBytes = volumeSize.x * volumeSize.y * volumeSize.z * texDesc.bytesPP;

        int size = std::min(data_size, volumeNumBytes);
        if (texDesc.type == GL_UNSIGNED_BYTE) {
            data = readModifyData<int8_t, uint8_t, uint8_t>(rawData, size, [](const uint8_t& val){ return val; });
        } else if (texDesc.type == GL_UNSIGNED_SHORT) {
            auto l_scaleValue = scaleValue;
            data = readModifyData<int8_t, uint16_t, uint16_t>(rawData, size, [l_scaleValue](const uint16_t& val){ return val * l_scaleValue; });
            /*auto elementSize = static_cast<unsigned int>(sizeof(uint16_t));
            auto size = std::min(data_size, volumeNumBytes) / elementSize;
            auto ptr = reinterpret_cast<uint16_t*>(rawData.data());
            for (unsigned int i = 0; i < size; ++i) {
                reinterpret_cast<uint16_t*>(data.data())[i] = ptr[i] * scaleValue;
            }*/
        } else if (texDesc.type == GL_UNSIGNED_INT) {
            data = readModifyData<int8_t, uint32_t, uint32_t>(rawData, size, [](const uint32_t& val){ return val; });
            /*auto elementSize = static_cast<unsigned int>(sizeof(uint32_t));
            auto size = std::min(data_size, volumeNumBytes) / elementSize;
            auto ptr = reinterpret_cast<uint32_t*>(rawData.data());
            for (unsigned int i = 0; i < size; ++i) {
                reinterpret_cast<uint32_t*>(data.data())[i] = ptr[i];
            }*/
        }

        auto volTex = std::make_unique<GLTexture>(volumeSize.x, volumeSize.y, volumeSize.z, mipLevels, texDesc, data.data());
        volTex->GenerateMipMaps();
        return std::move(volTex);
    }

    void Volume::Unload()
    {
        Resource::Unload();
    }

    std::unique_ptr<MinMaxVolume> Volume::GetMinMaxTexture() const
    {
        assert(texDesc.format == GL_RED);

        auto baseFileName = id.substr(0, id.find_last_of("."));

        auto newBaseFileName = baseFileName + "_minmax";
        auto newFilename = application->GetConfig().resourceBase + "/" + newBaseFileName + ".dat";
        auto path = newFilename.substr(0, newFilename.find_last_of("/\\"));

        if (!boost::filesystem::exists(newFilename)) {
            std::ofstream datOut(newFilename, std::ofstream::out | std::ofstream::trunc);
            if (!datOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newFilename) << "'.";
                throw std::runtime_error("Could not open file '" + newFilename + "'.");
            }

            auto baseRawFileName = rawFileName.substr(path.size() + 1);
            baseRawFileName = baseRawFileName.substr(0, baseRawFileName.find_last_of("."));
            auto newRawFileName = baseRawFileName + "_minmax.raw";

            std::string newFormat = "UCHAR";
            if (texDesc.type == GL_UNSIGNED_SHORT) newFormat = "USHORT";
            else if (texDesc.type == GL_UNSIGNED_INT) newFormat = "UINT";

            datOut << "ObjectFileName:\t" << newRawFileName << std::endl;
            datOut << "Resolution:\t" << volumeSize.x << " " << volumeSize.y << " " << volumeSize.z << std::endl;
            datOut << "SliceThickness:\t" << cellSize.x << " " << cellSize.y << " " << cellSize.z << std::endl;
            datOut << "Format:\t" << newFormat << std::endl;
            datOut << "ObjectModel:\tRGBA" << std::endl;

            datOut.close();

            std::fstream rawOut(application->GetConfig().resourceBase + "/" + newRawFileName,
                std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            if (!rawOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newRawFileName) << "'.";
                throw std::runtime_error("Could not open file '" + newRawFileName + "'.");
            }

            GPUProgram* minMaxProg = nullptr;
            TextureDescriptor minMaxDesc{ texDesc.bytesPP * 4, 0, GL_RGBA, texDesc.type };
            if (texDesc.type == GL_UNSIGNED_BYTE) {
                minMaxDesc.internalFormat = GL_RGBA16F;
                minMaxProg = application->GetGPUProgramManager()->GetResource("genMinMaxTexture16.cp");
            } else if (texDesc.type == GL_UNSIGNED_SHORT) {
                minMaxDesc.internalFormat = GL_RGBA16F;
                minMaxProg = application->GetGPUProgramManager()->GetResource("genMinMaxTexture16.cp");
            } else if (texDesc.type == GL_UNSIGNED_INT) {
                minMaxDesc.internalFormat = GL_RGBA32F;
                minMaxProg = application->GetGPUProgramManager()->GetResource("genMinMaxTexture32.cp");
            } else throw std::runtime_error("Pixel-type not supported.");
            auto uniformNames = minMaxProg->GetUniformLocations(boost::assign::list_of<std::string>("origTex")("minMaxTex"));

            minMaxProg->UseProgram();
            minMaxProg->SetUniform(uniformNames[0], 0);
            minMaxProg->SetUniform(uniformNames[1], 1);

            glm::uvec3 chunkSize(256);
            glm::uvec3 chunkPos(0);
            for (; chunkPos.z < volumeSize.z; chunkPos.z += chunkSize.z) {
                for (; chunkPos.y < volumeSize.y; chunkPos.y += chunkSize.y) {
                    for (; chunkPos.x < volumeSize.x; chunkPos.x += chunkSize.x) {
                        std::vector<uint8_t> chunkData;
                        auto dataSize = glm::min(chunkSize, volumeSize - chunkPos);
                        ReadRaw(chunkData, chunkPos, dataSize, dataSize);

                        GLTexture chunkTex(dataSize.x, dataSize.y, dataSize.z, 1, texDesc, chunkData.data());
                        GLTexture mmChunkTex(dataSize.x, dataSize.y, dataSize.z, 1, minMaxDesc, nullptr);

                        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(dataSize) / 8.0f));
                        chunkTex.ActivateImage(0, 0, GL_READ_ONLY);
                        mmChunkTex.ActivateImage(1, 0, GL_WRITE_ONLY);
                        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                        OGL_SCALL(glFinish);

                        std::vector<uint8_t> mmChunkData;
                        mmChunkTex.DownloadData(mmChunkData);

                        WriteRaw(mmChunkData, rawOut, chunkPos, dataSize, volumeSize, minMaxDesc.bytesPP);
                    }
                    chunkPos.x = 0;
                }
                chunkPos.y = 0;
            }
            rawOut.close();
        }

        return application->GetVolumeManager()->GetResource(newBaseFileName + ".dat");
    }

    /*Volume* Volume::GetHalfResTexture(bool denoise) const
    {
        assert(texDesc.format == GL_RGBA);
        auto maxSize = glm::max(glm::max(volumeSize.x, volumeSize.y), volumeSize.z);
        if (maxSize % 2 != 0) maxSize += 1;

        auto baseFileName = id.substr(0, id.find_last_of("."));
        std::stringstream dsEnding;
        dsEnding << "_downscale" << maxSize;

        if (boost::algorithm::ends_with(baseFileName, dsEnding.str())) {
            baseFileName = baseFileName.substr(0, baseFileName.find_last_of("_"));
        }

        std::stringstream newdsEnding;
        auto newMaxSize = maxSize >> 1;
        if (newMaxSize % 2 != 0) newMaxSize += 1;
        newdsEnding << "_downscale" << newMaxSize;
        auto newBaseFileName = baseFileName + newdsEnding.str();
        auto newFilename = application->GetConfig().resourceBase + "/" + newBaseFileName + ".dat";
        auto path = newFilename.substr(0, newFilename.find_last_of("/\\"));

        if (!boost::filesystem::exists(newFilename)) {
            std::ofstream datOut(newFilename, std::ofstream::out | std::ofstream::trunc);
            if (!datOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newFilename) << "'.";
                throw std::runtime_error("Could not open file '" + newFilename + "'.");
            }

            auto baseRawFileName = rawFileName.substr(path.size() + 1);
            baseRawFileName = baseRawFileName.substr(0, baseRawFileName.find_last_of("."));
            if (boost::algorithm::ends_with(baseRawFileName, dsEnding.str())) {
                baseRawFileName = baseRawFileName.substr(0, baseRawFileName.find_last_of("_"));
            }

            auto newVolumeSize = volumeSize;
            if (newVolumeSize.x % 2 != 0) newVolumeSize.x += 1;
            if (newVolumeSize.y % 2 != 0) newVolumeSize.y += 1;
            if (newVolumeSize.z % 2 != 0) newVolumeSize.z += 1;
            newVolumeSize >>= glm::uvec3(1);
            auto newRawFileName = baseRawFileName + newdsEnding.str() + ".raw";

            std::string newFormat = "UCHAR";
            if (texDesc.type == GL_UNSIGNED_SHORT) newFormat = "USHORT";
            else if (texDesc.type == GL_UNSIGNED_INT) newFormat = "UINT";

            std::string newObjModel = "I";
            if (texDesc.format == GL_RG) newObjModel = "RG";
            else if (texDesc.format == GL_RGB) newObjModel = "RGB";
            else if (texDesc.format == GL_RGBA) newObjModel = "RGBA";

            datOut << "ObjectFileName:\t" << newRawFileName << std::endl;
            datOut << "Resolution:\t" << newVolumeSize.x << " " << newVolumeSize.y << " " << newVolumeSize.z << std::endl;
            datOut << "SliceThickness:\t" << cellSize.x << " " << cellSize.y << " " << cellSize.z << std::endl;
            datOut << "Format:\t" << newFormat << std::endl;
            datOut << "ObjectModel:\t" << newObjModel << std::endl;

            datOut.close();

            std::fstream rawOut(application->GetConfig().resourceBase + "/" + newRawFileName,
                std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            if (!rawOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newRawFileName) << "'.";
                throw std::runtime_error("Could not open file '" + newRawFileName + "'.");
            }

            GPUProgram* downSizeProg = nullptr;
            if (!denoise) {
                if (texDesc.type == GL_UNSIGNED_BYTE) downSizeProg = application->GetGPUProgramManager()->GetResource("downsize3DTex16.cp");
                else if (texDesc.type == GL_UNSIGNED_SHORT) downSizeProg = application->GetGPUProgramManager()->GetResource("downsize3DTex16.cp");
                else if (texDesc.type == GL_UNSIGNED_INT) downSizeProg = application->GetGPUProgramManager()->GetResource("downsize3DTex32.cp");
                else throw std::runtime_error("Texture bit depth is not supported for min/max octrees.");
            } else {
                if (texDesc.type == GL_UNSIGNED_BYTE) downSizeProg = application->GetGPUProgramManager()->GetResource("downsizeDenoise3DTex16.cp");
                else if (texDesc.type == GL_UNSIGNED_SHORT) downSizeProg = application->GetGPUProgramManager()->GetResource("downsizeDenoise3DTex16.cp");
                else if (texDesc.type == GL_UNSIGNED_INT) downSizeProg = application->GetGPUProgramManager()->GetResource("downsizeDenoise3DTex32.cp");
                else throw std::runtime_error("Texture bit depth is not supported for min/max octrees.");
            }
            auto uniformNames = downSizeProg->GetUniformLocations(boost::assign::list_of<std::string>("origTex")("downsizedTex"));

            downSizeProg->UseProgram();
            downSizeProg->SetUniform(uniformNames[0], 0);
            downSizeProg->SetUniform(uniformNames[1], 1);

            glm::uvec3 chunkSize(256);
            glm::uvec3 chunkPos(0);
            for (; chunkPos.z < volumeSize.z; chunkPos.z += chunkSize.z) {
                for (; chunkPos.y < volumeSize.y; chunkPos.y += chunkSize.y) {
                    for (; chunkPos.x < volumeSize.x; chunkPos.x += chunkSize.x) {
                        std::vector<uint8_t> chunkData;
                        auto dataSize = glm::min(chunkSize, volumeSize - chunkPos);
                        auto texSize = glm::min(chunkSize, (newVolumeSize << glm::uvec3(1)) - chunkPos);
                        auto smallTexSize = texSize >> glm::uvec3(1);
                        ReadRaw(chunkData, chunkPos, dataSize, texSize);

                        GLTexture chunkTex(texSize.x, texSize.y, texSize.z, 1, texDesc, chunkData.data());
                        GLTexture smallChunkTex(smallTexSize.x, smallTexSize.y, smallTexSize.z, 1, texDesc, nullptr);

                        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(smallTexSize) / 8.0f));
                        chunkTex.ActivateImage(0, 0, GL_READ_ONLY);
                        smallChunkTex.ActivateImage(1, 0, GL_WRITE_ONLY);
                        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                        OGL_SCALL(glFinish);

                        std::vector<uint8_t> smallChunkData;
                        smallChunkTex.DownloadData(smallChunkData);

                        WriteRaw(smallChunkData, rawOut, chunkPos >> glm::uvec3(1), smallTexSize, newVolumeSize, texDesc.bytesPP);
                    }
                    chunkPos.x = 0;
                }
                chunkPos.y = 0;
            }
            rawOut.close();
        }

        return application->GetVolumeManager()->GetResource(newBaseFileName + ".dat");
    }*/

    Volume* Volume::GetSpeedVolume() const
    {
        assert(texDesc.format == GL_RGBA);

        auto baseFileName = id.substr(0, id.find_last_of("."));
        auto newBaseFileName = baseFileName + "_speed";
        auto newFilename = application->GetConfig().resourceBase + "/" + newBaseFileName + ".dat";
        auto path = newFilename.substr(0, newFilename.find_last_of("/\\"));

        if (!boost::filesystem::exists(newFilename)) {
            std::ofstream datOut(newFilename, std::ofstream::out | std::ofstream::trunc);
            if (!datOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newFilename) << "'.";
                throw std::runtime_error("Could not open file '" + newFilename + "'.");
            }

            auto baseRawFileName = rawFileName.substr(path.size() + 1);
            baseRawFileName = baseRawFileName.substr(0, baseRawFileName.find_last_of("."));
            auto newRawFileName = baseRawFileName + "_speed.raw";

            std::string newFormat = "UCHAR";
            if (texDesc.type == GL_UNSIGNED_SHORT) newFormat = "USHORT";
            else if (texDesc.type == GL_UNSIGNED_INT) newFormat = "UINT";

            datOut << "ObjectFileName:\t" << newRawFileName << std::endl;
            datOut << "Resolution:\t" << volumeSize.x << " " << volumeSize.y << " " << volumeSize.z << std::endl;
            datOut << "SliceThickness:\t" << cellSize.x << " " << cellSize.y << " " << cellSize.z << std::endl;
            datOut << "Format:\t" << newFormat << std::endl;
            datOut << "ObjectModel:\tI" << std::endl;

            datOut.close();

            std::fstream rawOut(application->GetConfig().resourceBase + "/" + newRawFileName,
                std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            if (!rawOut.is_open()) {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                LOG(ERROR) << "Could not open file '" << converter.from_bytes(newRawFileName) << "'.";
                throw std::runtime_error("Could not open file '" + newRawFileName + "'.");
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

        return application->GetVolumeManager()->GetResource(newBaseFileName + ".dat");
    }

    /*std::unique_ptr<VolumeBrickOctree> GLTexture3D::GetBrickedVolume(const glm::vec3& scale, int denoiseLevel) const
    {
        assert(texDesc.format == GL_RED || texDesc.format == GL_RED_INTEGER);
        std::ifstream ifs(rawFileName, std::ios::binary);
        if (!ifs.is_open()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Could not open file '" << converter.from_bytes(rawFileName) << "'.";
            throw std::runtime_error("Could not open file '" + rawFileName + "'.");
        }

        GPUProgram* minMaxProg;
        // TODO: change shaders. with defines only one shader is needed. [1/6/2016 Sebastian Maisch]
        if (texDesc.bytesPP == 1) minMaxProg = application->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMaxMipMaps8.cp");
        else if (texDesc.bytesPP == 2) minMaxProg = application->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMaxMipMaps16.cp");
        else if (texDesc.bytesPP == 4) minMaxProg = application->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMaxMipMaps32.cp");
        else throw std::runtime_error("Texture bit depth is not supported for min/max octrees.");
        auto uniformNames = minMaxProg->GetUniformLocations(boost::assign::list_of<std::string>("origTex")("nextLevelTex"));

        auto minMaxTex = GetMinMaxTexture();
        std::unique_ptr<VolumeBrickOctree> result{ new VolumeBrickOctree(minMaxTex, glm::uvec3(0), volumeSize,
            scale * cellSize, denoiseLevel, minMaxProg, uniformNames) };

        return std::move(result);
    }*/

    /*void GLTexture3D::ReadRaw(std::vector<uint8_t>& data, const glm::uvec3& pos, const glm::uvec3& dataSize,
        const glm::uvec3& texSize) const
    {
        data.resize(texSize.x * texSize.y * texSize.z * texDesc.bytesPP, 0);
        auto lineSize = volumeSize.x * texDesc.bytesPP;
        auto dataPtr = reinterpret_cast<char*>(data.data());

        std::ifstream fileStream(rawFileName, std::ios::binary | std::ios::in);
        if (!fileStream.is_open()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Could not open file '" << converter.from_bytes(rawFileName) << "'.";
            throw std::runtime_error("Could not open file '" + rawFileName + "'.");
        }

        for (unsigned int z = 0; z < dataSize.z; ++z) {
            for (unsigned int y = 0; y < dataSize.y; ++y) {
                auto lineStartFile = ((pos.z + z) * volumeSize.y * lineSize) + ((pos.y + y) * lineSize) + pos.x * texDesc.bytesPP;
                fileStream.seekg(lineStartFile, std::ios::beg);
                fileStream.read(dataPtr, dataSize.x * texDesc.bytesPP);
                dataPtr += texSize.x * texDesc.bytesPP;
            }
        }
        fileStream.close();

        if (scaleValue != 1) {
            if (texDesc.type == GL_UNSIGNED_BYTE) {
                for (auto& dt : data) {
                    dt *= static_cast<unsigned char>(scaleValue);
                }
            } else if (texDesc.type == GL_UNSIGNED_SHORT) {
                auto elementSize = static_cast<unsigned int>(sizeof(uint16_t));
                auto size = static_cast<unsigned int>(data.size()) / elementSize;
                auto ptr = reinterpret_cast<uint16_t*>(data.data());
                for (unsigned int i = 0; i < size; ++i) {
                    ptr[i] *= static_cast<uint16_t>(scaleValue);
                }
            } else if (texDesc.type == GL_UNSIGNED_INT) {
                auto elementSize = static_cast<unsigned int>(sizeof(uint32_t));
                auto size = static_cast<unsigned int>(data.size()) / elementSize;
                auto ptr = reinterpret_cast<uint32_t*>(data.data());
                for (unsigned int i = 0; i < size; ++i) {
                    ptr[i] *= scaleValue;
                }
            }
        }
    }*/

    /**
     *  Writes data in 3D chunks to a file.
     *  @param data the raw data of the chunk to write.
     *  @param fileStream the stream to write to.
     *  @param pos the position inside the file to write to.
     *  @param dataSize the dimensions of the data.
     *  @param volumeSize the dimensions of the complete volume.
     *  @param bytesPV the number of bytes per voxel.
     */
    /*void GLTexture3D::WriteRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
        const glm::uvec3& dataSize, const glm::uvec3& volumeSize, unsigned int bytesPV)
    {
        auto fileLineSize = volumeSize.x * bytesPV;
        auto dataPtr = reinterpret_cast<char*>(data.data());

        for (unsigned int z = 0; z < dataSize.z; ++z) {
            for (unsigned int y = 0; y < dataSize.y; ++y) {
                auto lineStartFile = ((pos.z + z) * volumeSize.y * fileLineSize)
                    + ((pos.y + y) * fileLineSize)
                    + pos.x * bytesPV;
                fileStream.seekp(lineStartFile, std::ios::beg);
                fileStream.write(dataPtr, dataSize.x * bytesPV);
                dataPtr += dataSize.x * bytesPV;
            }
        }
    }*/
}
