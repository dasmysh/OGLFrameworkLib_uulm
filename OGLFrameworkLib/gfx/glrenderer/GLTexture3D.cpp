/**
 * @file   GLTexture3D.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.14
 *
 * @brief  Contains the implementation of GLTexture3D.
 */

#include "GLTexture3D.h"
#include "app/ApplicationBase.h"
#include <codecvt>
#include <fstream>
#include "GLTexture.h"
#include <boost/assign.hpp>
#include "gfx/volumes/VolumeBrickOctree.h"
#include <ios>
#include <boost/filesystem.hpp>
#include "app/Configuration.h"
#include <boost/algorithm/string/predicate.hpp>

#undef min
#undef max

namespace cgu {

    /**
     * Constructor.
     * @param texFilename the textures file name
     * @param app the application object
     */
    GLTexture3D::GLTexture3D(const std::string& texFilename, ApplicationBase* app) :
        Resource{ texFilename, app },
        texture(),
        volumeSize(0),
        cellSize(1.0f),
        scaleValue(1),
        dataDim(1),
        texDesc(4, GL_R8, GL_RED, GL_UNSIGNED_BYTE)
    {
    }

    /** Copy constructor. */
    GLTexture3D::GLTexture3D(const GLTexture3D& rhs) : GLTexture3D(rhs.id, rhs.application)
    {
        if (rhs.IsLoaded()) GLTexture3D::Load();
    }

    /** Copy assignment operator. */
    GLTexture3D& GLTexture3D::operator=(const GLTexture3D& rhs)
    {
        if (this != &rhs) {
            auto tmp(rhs);
            std::swap(texture, tmp.texture);
            std::swap(volumeSize, tmp.volumeSize);
            std::swap(cellSize, tmp.cellSize);
            std::swap(rawFileName, tmp.rawFileName);
            std::swap(scaleValue, tmp.scaleValue);
            std::swap(dataDim, tmp.dataDim);
            std::swap(texDesc, tmp.texDesc);
            std::swap(data, tmp.data);
        }
        return *this;
    }

    /** Move constructor. */
    GLTexture3D::GLTexture3D(GLTexture3D&& rhs) :
        Resource(std::move(rhs)),
        texture(std::move(rhs.texture)),
        volumeSize(std::move(rhs.volumeSize)),
        cellSize(std::move(rhs.cellSize)),
        rawFileName(std::move(rhs.rawFileName)),
        scaleValue(std::move(rhs.scaleValue)),
        dataDim(std::move(rhs.dataDim)),
        texDesc(std::move(rhs.texDesc)),
        data(std::move(rhs.data))
    {
        
    }
    GLTexture3D& GLTexture3D::operator=(GLTexture3D&& rhs)
    {
        this->~GLTexture3D();
        texture = std::move(rhs.texture);
        volumeSize = std::move(rhs.volumeSize);
        cellSize = std::move(rhs.cellSize);
        rawFileName = std::move(rhs.rawFileName);
        scaleValue = std::move(rhs.scaleValue);
        dataDim = std::move(rhs.dataDim);
        texDesc = std::move(rhs.texDesc);
        data = std::move(rhs.data);
        return *this;
    }

    /** Destructor. */
    GLTexture3D::~GLTexture3D()
    {
        if (IsLoaded()) UnloadLocal();
    }

    void GLTexture3D::Load()
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

        Resource::Load();
    }

    /**
    * Unloads the local resources.
    */
    void GLTexture3D::UnloadLocal()
    {
        texture.reset();
        data.clear();
    }

    /**
     *  Loads the content of the volume to a single texture.
     *  @param mipLevels the number of MipMap levels the texture should have.
     *  @return the loaded texture.
     */
    GLTexture* GLTexture3D::LoadToSingleTexture(unsigned int mipLevels)
    {
        std::ifstream ifsRaw(rawFileName, std::ios::in | std::ios::binary);
        if (!ifsRaw || !ifsRaw.is_open()) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << "Could not open file '" << converter.from_bytes(rawFileName) << "'.";
            throw std::runtime_error("Could not open file '" + rawFileName + "'.");
        }

        ifsRaw.seekg(0, std::ios::end);
        auto data_size = static_cast<unsigned int>(ifsRaw.tellg());
        ifsRaw.seekg(0, std::ios::beg);

        std::vector<char> rawData(data_size, 9);
        ifsRaw.read(rawData.data(), data_size);
        ifsRaw.close();

        auto volumeNumBytes = volumeSize.x * volumeSize.y * volumeSize.z * texDesc.bytesPP;
        data.resize(volumeNumBytes);

        if (texDesc.type == GL_UNSIGNED_BYTE) {
            int size = std::min(data_size, volumeNumBytes);
            auto ptr = reinterpret_cast<uint8_t*>(rawData.data());
            for (auto i = 0; i < size; ++i) {
                data[i] = ptr[i];
            }
        } else if (texDesc.type == GL_UNSIGNED_SHORT) {
            auto elementSize = static_cast<unsigned int>(sizeof(uint16_t));
            auto size = std::min(data_size, volumeNumBytes) / elementSize;
            auto ptr = reinterpret_cast<uint16_t*>(rawData.data());
            for (unsigned int i = 0; i < size; ++i) {
                reinterpret_cast<uint16_t*>(data.data())[i] = ptr[i] * scaleValue;
            }
        } else if (texDesc.type == GL_UNSIGNED_INT) {
            auto elementSize = static_cast<unsigned int>(sizeof(uint32_t));
            auto size = std::min(data_size, volumeNumBytes) / elementSize;
            auto ptr = reinterpret_cast<uint32_t*>(rawData.data());
            for (unsigned int i = 0; i < size; ++i) {
                reinterpret_cast<uint32_t*>(data.data())[i] = ptr[i];
            }
        }

        texture = std::make_unique<GLTexture>(volumeSize.x, volumeSize.y, volumeSize.z, mipLevels, texDesc, data.data());

        return texture.get();
    }

    /** Returns the texture object. */
    GLTexture* GLTexture3D::GetTexture() const
    {
        return texture.get();
    }

    void GLTexture3D::Unload()
    {
        UnloadLocal();
        Resource::Unload();
    }

    GLTexture3D* GLTexture3D::GetMinMaxTexture() const
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

    GLTexture3D* GLTexture3D::GetHalfResTexture(bool denoise) const
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
    }

    GLTexture3D* GLTexture3D::GetSpeedImage() const
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

            auto newTexDesc = texDesc;
            auto texDesc2 = texDesc;
            newTexDesc.bytesPP = texDesc.bytesPP / 4;
            newTexDesc.format = GL_RED;
            newTexDesc.type = texDesc.type;
            texDesc2.internalFormat = GL_RGBA16F;

            GPUProgram* speedProg = nullptr;
            if (texDesc.type == GL_UNSIGNED_BYTE) {
                speedProg = application->GetGPUProgramManager()->GetResource("speed3DTex16.cp");
                newTexDesc.internalFormat = GL_R16F;
            } else if (texDesc.type == GL_UNSIGNED_SHORT) {
                speedProg = application->GetGPUProgramManager()->GetResource("speed3DTex16.cp");
                newTexDesc.internalFormat = GL_R16F;
            } else if (texDesc.type == GL_UNSIGNED_INT) {
                speedProg = application->GetGPUProgramManager()->GetResource("speed3DTex32.cp");
                newTexDesc.internalFormat = GL_R32F;
            } else throw std::runtime_error("Texture bit depth is not supported for min/max octrees.");
            auto uniformNames = speedProg->GetUniformLocations(boost::assign::list_of<std::string>("origTex")("speedTex"));

            speedProg->UseProgram();
            speedProg->SetUniform(uniformNames[0], 0);
            speedProg->SetUniform(uniformNames[1], 1);

            glm::uvec3 chunkSize(256);
            glm::uvec3 chunkPos(0);
            for (; chunkPos.z < volumeSize.z; chunkPos.z += chunkSize.z) {
                for (; chunkPos.y < volumeSize.y; chunkPos.y += chunkSize.y) {
                    for (; chunkPos.x < volumeSize.x; chunkPos.x += chunkSize.x) {
                        std::vector<uint8_t> chunkData;
                        auto dataSize = glm::min(chunkSize, volumeSize - chunkPos);
                        ReadRaw(chunkData, chunkPos, dataSize, dataSize);

                        GLTexture chunkTex(dataSize.x, dataSize.y, dataSize.z, 1, texDesc2, chunkData.data());
                        GLTexture speedChunkTex(dataSize.x, dataSize.y, dataSize.z, 1, newTexDesc, nullptr);

                        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(dataSize) / 8.0f));
                        chunkTex.ActivateImage(0, 0, GL_READ_ONLY);
                        speedChunkTex.ActivateImage(1, 0, GL_WRITE_ONLY);
                        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                        OGL_SCALL(glFinish);

                        std::vector<uint8_t> speedChunkData;
                        speedChunkTex.DownloadData(speedChunkData);

                        WriteRaw(speedChunkData, rawOut, chunkPos, dataSize, volumeSize, newTexDesc.bytesPP);
                    }
                    chunkPos.x = 0;
                }
                chunkPos.y = 0;
            }
            rawOut.close();
        }

        return application->GetVolumeManager()->GetResource(newBaseFileName + ".dat");
    }

    std::unique_ptr<VolumeBrickOctree> GLTexture3D::GetBrickedVolume(const glm::vec3& scale, int denoiseLevel) const
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
    }

    void GLTexture3D::ReadRaw(std::vector<uint8_t>& data, const glm::uvec3& pos, const glm::uvec3& dataSize,
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
    }

    /**
     *  Writes data in 3D chunks to a file.
     *  @param data the raw data of the chunk to write.
     *  @param fileStream the stream to write to.
     *  @param pos the position inside the file to write to.
     *  @param dataSize the dimensions of the data.
     *  @param volumeSize the dimensions of the complete volume.
     *  @param bytesPV the number of bytes per voxel.
     */
    void GLTexture3D::WriteRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
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
    }

    glm::uvec3 GLTexture3D::GetBrickTextureSize(const glm::uvec3& pos, const glm::uvec3& size) const
    {
        auto dataSize = glm::min(size, volumeSize - pos);
        glm::uvec3 actualSize;
        if (dataSize.x == 1) actualSize.x = 2;
        else actualSize.x = cguMath::roundupPow2(dataSize.x);
        if (dataSize.y == 1) actualSize.y = 2;
        else actualSize.y = cguMath::roundupPow2(dataSize.y);
        if (dataSize.z == 1) actualSize.z = 2;
        else actualSize.z = cguMath::roundupPow2(dataSize.z);
        return std::move(actualSize);
    }

    const TextureDescriptor& GLTexture3D::GetTextureDescriptor() const
    {
        return texDesc;
    }
}
