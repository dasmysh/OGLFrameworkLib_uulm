/**
 * @file   GLVolumeInitializer.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.09.21
 *
 * @brief  
 */

#include "GLVolumeInitializer.h"
#include "app/ApplicationBase.h"
#include "app/Configuration.h"
#include <fstream>
#include <codecvt>
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>

namespace cgu {
    namespace gpgpu {

        GLVolumeInitializer::GLVolumeInitializer(unsigned int width, unsigned int height, unsigned int depth,
            const TextureDescriptor& desc) :
            volSize(width, height, depth),
            texDesc(desc)
        {
        }


        GLVolumeInitializer::~GLVolumeInitializer()
        {
        }

        Volume* GLVolumeInitializer::InitChecker(const std::string& filename, const glm::uvec3& checkerSize, ApplicationBase* app) const
        {
            auto initProg = app->GetGPUProgramManager()->GetResource("synthChecker.cp");
            auto uniformNames = initProg->GetUniformLocations(boost::assign::list_of<std::string>("resultImg")("checkerSize")("offset"));
            return InitGeneral(filename, initProg, uniformNames, app, [&](const glm::uvec3& chunkPos,
                const glm::uvec3& dataSize, const TextureDescriptor& internalTexDesc, std::fstream& rawOut){

                GLTexture resultTex(dataSize.x, dataSize.y, dataSize.z, 1, internalTexDesc, nullptr);

                                   auto numGroups = glm::ivec3(glm::ceil(glm::vec3(dataSize) / 8.0f));
                resultTex.ActivateImage(0, 0, GL_WRITE_ONLY);
                initProg->SetUniform(uniformNames[1], checkerSize);
                initProg->SetUniform(uniformNames[2], chunkPos);
                OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                OGL_SCALL(glFinish);

                std::vector<uint8_t> resultData;
                resultTex.DownloadData(resultData);

                WriteRaw(resultData, rawOut, chunkPos, dataSize, volSize, texDesc.bytesPP);
            });
            /*assert(texDesc.format == GL_RED);

            std::string baseFileName = filename.substr(0, filename.find_last_of("."));
            std::string newFilename = app->GetConfig().resourceBase + "/" + baseFileName + ".dat";
            std::string path = newFilename.substr(0, newFilename.find_last_of("/\\"));

            if (!boost::filesystem::exists(newFilename)) {
                std::ofstream datOut(newFilename, std::ofstream::out | std::ofstream::trunc);
                if (!datOut.is_open()) {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    LOG(ERROR) << "Could not open file '" << converter.from_bytes(newFilename) << "'.";
                    throw std::runtime_error("Could not open file '" + newFilename + "'.");
                }

                std::string rawFileName = baseFileName + ".raw";

                std::string newFormat = "UCHAR";
                if (texDesc.type == GL_UNSIGNED_SHORT) newFormat = "USHORT";
                else if (texDesc.type == GL_UNSIGNED_INT) newFormat = "UINT";

                datOut << "ObjectFileName:\t" << rawFileName << std::endl;
                datOut << "Resolution:\t" << volSize.x << " " << volSize.y << " " << volSize.z << std::endl;
                datOut << "SliceThickness:\t" << 1 << " " << 1 << " " << 1 << std::endl;
                datOut << "Format:\t" << newFormat << std::endl;
                datOut << "ObjectModel:\tI" << std::endl;

                datOut.close();

                std::fstream rawOut(app->GetConfig().resourceBase + "/" + rawFileName,
                    std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                if (!rawOut.is_open()) {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    LOG(ERROR) << "Could not open file '" << converter.from_bytes(rawFileName) << "'.";
                    throw std::runtime_error("Could not open file '" + rawFileName + "'.");
                }

                TextureDescriptor internalTexDesc = texDesc;
                internalTexDesc.internalFormat = GL_R32F;

                GPUProgram* initProg = app->GetGPUProgramManager()->GetResource("synthChecker.cp");
                std::vector<BindingLocation> uniformNames = initProg->GetUniformLocations(boost::assign::list_of<std::string>("resultImg")("checkerSize")("offset"));

                initProg->UseProgram();
                initProg->SetUniform(uniformNames[0], 0);

                glm::uvec3 chunkSize(256);
                glm::uvec3 chunkPos(0);
                for (; chunkPos.z < volSize.z; chunkPos.z += chunkSize.z) {
                    for (; chunkPos.y < volSize.y; chunkPos.y += chunkSize.y) {
                        for (; chunkPos.x < volSize.x; chunkPos.x += chunkSize.x) {
                            glm::uvec3 dataSize = glm::min(chunkSize, volSize - chunkPos);
                            GLTexture resultTex(dataSize.x, dataSize.y, dataSize.z, 1, internalTexDesc, nullptr);

                            glm::ivec3 numGroups = glm::ivec3(glm::ceil(glm::vec3(dataSize) / 8.0f));
                            resultTex.ActivateImage(0, 0, GL_WRITE_ONLY);
                            initProg->SetUniform(uniformNames[1], checkerSize);
                            initProg->SetUniform(uniformNames[2], chunkPos);
                            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                            OGL_SCALL(glFinish);

                            std::vector<uint8_t> resultData;
                            resultTex.DownloadData(resultData);

                            WriteRaw(resultData, rawOut, chunkPos, dataSize, volSize, texDesc.bytesPP);
                        }
                        chunkPos.x = 0;
                    }
                    chunkPos.y = 0;
                }
                rawOut.close();
            }

            return app->GetVolumeManager()->GetResource(baseFileName + ".dat");*/
        }

        Volume* GLVolumeInitializer::InitStripes(const std::string& filename, unsigned int stripeSize, ApplicationBase* app) const
        {
            auto initProg = app->GetGPUProgramManager()->GetResource("synthStripes.cp");
            auto uniformNames = initProg->GetUniformLocations(boost::assign::list_of<std::string>("resultImg")("stripeSize")("offset"));
            return InitGeneral(filename, initProg, uniformNames, app, [&](const glm::uvec3& chunkPos,
                const glm::uvec3& dataSize, const TextureDescriptor& internalTexDesc, std::fstream& rawOut){

                GLTexture resultTex(dataSize.x, dataSize.y, dataSize.z, 1, internalTexDesc, nullptr);

                                   auto numGroups = glm::ivec3(glm::ceil(glm::vec3(dataSize) / 8.0f));
                resultTex.ActivateImage(0, 0, GL_WRITE_ONLY);
                initProg->SetUniform(uniformNames[1], glm::uvec3(stripeSize));
                initProg->SetUniform(uniformNames[2], chunkPos);
                OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                OGL_SCALL(glFinish);

                std::vector<uint8_t> resultData;
                resultTex.DownloadData(resultData);

                WriteRaw(resultData, rawOut, chunkPos, dataSize, volSize, texDesc.bytesPP);
            });
        }


        Volume* GLVolumeInitializer::InitSpherical(const std::string& filename, const glm::vec3& sphereCenter,
            const glm::vec3& sphereScale, ApplicationBase* app) const
        {
            auto initProg = app->GetGPUProgramManager()->GetResource("synthSpherical.cp");
            auto uniformNames = initProg->GetUniformLocations(boost::assign::list_of<std::string>("resultImg")("sphereCenter")("sphereScale")("offset"));
            return InitGeneral(filename, initProg, uniformNames, app, [&](const glm::uvec3& chunkPos,
                const glm::uvec3& dataSize, const TextureDescriptor& internalTexDesc, std::fstream& rawOut){

                GLTexture resultTex(dataSize.x, dataSize.y, dataSize.z, 1, internalTexDesc, nullptr);

                auto numGroups = glm::ivec3(glm::ceil(glm::vec3(dataSize) / 8.0f));
                resultTex.ActivateImage(0, 0, GL_WRITE_ONLY);
                initProg->SetUniform(uniformNames[1], sphereCenter);
                initProg->SetUniform(uniformNames[2], sphereScale);
                initProg->SetUniform(uniformNames[3], chunkPos);
                OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
                OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                OGL_SCALL(glFinish);

                std::vector<uint8_t> resultData;
                resultTex.DownloadData(resultData);

                WriteRaw(resultData, rawOut, chunkPos, dataSize, volSize, texDesc.bytesPP);
            });
        }


        Volume* GLVolumeInitializer::InitGeneral(const std::string& filename, GPUProgram* initProg,
            const std::vector<BindingLocation>& uniformNames, ApplicationBase* app,
            std::function<void(const glm::uvec3&, const glm::uvec3&, const TextureDescriptor&, std::fstream&)> chunkInitialize) const
        {
            assert(texDesc.format == GL_RED);

            auto baseFileName = filename.substr(0, filename.find_last_of("."));
            auto newFilename = app->GetConfig().resourceBase + "/" + baseFileName + ".dat";
            auto path = newFilename.substr(0, newFilename.find_last_of("/\\"));

            if (!boost::filesystem::exists(newFilename)) {
                std::ofstream datOut(newFilename, std::ofstream::out | std::ofstream::trunc);
                if (!datOut.is_open()) {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    LOG(ERROR) << "Could not open file '" << converter.from_bytes(newFilename) << "'.";
                    throw std::runtime_error("Could not open file '" + newFilename + "'.");
                }

                auto rawFileName = baseFileName + ".raw";

                std::string newFormat = "UCHAR";
                if (texDesc.type == GL_UNSIGNED_SHORT) newFormat = "USHORT";
                else if (texDesc.type == GL_UNSIGNED_INT) newFormat = "UINT";

                datOut << "ObjectFileName:\t" << rawFileName << std::endl;
                datOut << "Resolution:\t" << volSize.x << " " << volSize.y << " " << volSize.z << std::endl;
                datOut << "SliceThickness:\t" << 1 << " " << 1 << " " << 1 << std::endl;
                datOut << "Format:\t" << newFormat << std::endl;
                datOut << "ObjectModel:\tI" << std::endl;

                datOut.close();

                std::fstream rawOut(app->GetConfig().resourceBase + "/" + rawFileName,
                    std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                if (!rawOut.is_open()) {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    LOG(ERROR) << "Could not open file '" << converter.from_bytes(rawFileName) << "'.";
                    throw std::runtime_error("Could not open file '" + rawFileName + "'.");
                }

                auto internalTexDesc = texDesc;
                internalTexDesc.internalFormat = GL_R32F;

                initProg->UseProgram();
                initProg->SetUniform(uniformNames[0], 0);

                glm::uvec3 chunkSize(256);
                glm::uvec3 chunkPos(0);
                for (; chunkPos.z < volSize.z; chunkPos.z += chunkSize.z) {
                    for (; chunkPos.y < volSize.y; chunkPos.y += chunkSize.y) {
                        for (; chunkPos.x < volSize.x; chunkPos.x += chunkSize.x) {
                            auto dataSize = glm::min(chunkSize, volSize - chunkPos);
                            chunkInitialize(chunkPos, dataSize, internalTexDesc, rawOut);
                        }
                        chunkPos.x = 0;
                    }
                    chunkPos.y = 0;
                }
                rawOut.close();
            }

            return app->GetVolumeManager()->GetResource(baseFileName + ".dat");
        }

        void GLVolumeInitializer::ReadRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
            const glm::uvec3& dataSize, const glm::uvec3& volumeSize, unsigned int bytesPV)
        {
            data.resize(volumeSize.x * volumeSize.y * volumeSize.z * bytesPV, 0);
            auto lineSize = volumeSize.x * bytesPV;
            auto dataPtr = reinterpret_cast<char*>(data.data());

            for (unsigned int z = 0; z < dataSize.z; ++z) {
                for (unsigned int y = 0; y < dataSize.y; ++y) {
                    auto lineStartFile = ((pos.z + z) * volumeSize.y * lineSize) + ((pos.y + y) * lineSize) + pos.x * bytesPV;
                    fileStream.seekg(lineStartFile, std::ios::beg);
                    fileStream.read(dataPtr, dataSize.x * bytesPV);
                    dataPtr += volumeSize.x * bytesPV;
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
        void GLVolumeInitializer::WriteRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
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
    }
}
