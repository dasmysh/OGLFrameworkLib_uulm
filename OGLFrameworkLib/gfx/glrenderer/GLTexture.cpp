/**
 * @file   GLTexture.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.08
 *
 * @brief  Contains the implementation of GLTexture.
 */

#include "GLTexture.h"
#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image_write.h>

#undef min
#undef max

namespace cgu {

    /**
     * Constructor.
     * Creates a 2d array texture.
     * @param w the textures width
     * @param h the textures height
     * @param desc the textures format
     * @param arraySize the array size
     */
    GLTexture::GLTexture(unsigned int w, unsigned int h, unsigned int arraySize, const TextureDescriptor& desc) :
        id{ GL_TEXTURE_2D_ARRAY },
        descriptor(desc),
        width(w),
        height(h),
        depth(arraySize),
        mipMapLevels(1)
    {
        OGL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, id.textureId);
        OGL_CALL(glTexStorage3D, GL_TEXTURE_2D_ARRAY, mipMapLevels, descriptor.internalFormat, width, height, depth);
        OGL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, 0);
        InitSampling();
    }

    GLTexture::GLTexture(unsigned int size, const TextureDescriptor& desc) :
        id{ GL_TEXTURE_1D },
        descriptor(desc),
        width(size),
        height(1),
        depth(1),
        mipMapLevels(1)
    {
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexStorage1D, id.textureType, mipMapLevels, descriptor.internalFormat, width);
        OGL_CALL(glBindTexture, id.textureType, 0);
        InitSampling();
    }

    /**
     * Constructor.
     * Creates a 2d texture.
     * @param w the textures width
     * @param h the textures height
     * @param desc the textures format
     * @param data the textures data
     */
    GLTexture::GLTexture(unsigned int w, unsigned int h, const TextureDescriptor& desc, const void* data) :
        id{ GL_TEXTURE_2D },
        descriptor(desc),
        width(w),
        height(h),
        depth(1),
        mipMapLevels(1)
    {
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexStorage2D, id.textureType, mipMapLevels, descriptor.internalFormat, width, height);
        if (data) {
            OGL_CALL(glTexSubImage2D, id.textureType, 0, 0, 0, width, height, descriptor.format,
                descriptor.type, data);
        }
        OGL_CALL(glBindTexture, id.textureType, 0);
        InitSampling();
    }

    /**
    * Constructor.
    * Creates a 3d texture.
    * @param w the textures width
    * @param h the textures height
    * @param d the textures depth
    * @param numMipLevels the number of MipMap levels to create.
    * @param desc the textures format
    * @param data the textures data
    */
    GLTexture::GLTexture(unsigned int w, unsigned int h, unsigned int d, unsigned int numMipLevels, const TextureDescriptor& desc, const void* data) :
        id{ GL_TEXTURE_3D },
        descriptor(desc),
        width(w),
        height(h),
        depth(d),
        mipMapLevels(numMipLevels)
    {
        GLint maxSizeSystem;
        OGL_CALL(glGetIntegerv, GL_MAX_3D_TEXTURE_SIZE, &maxSizeSystem);
        auto maxSize = glm::max(glm::max(width, height), depth);

        if (maxSize > static_cast<unsigned int>(maxSizeSystem))
            throw std::runtime_error("Texture size is too big.");

        mipMapLevels = glm::min(mipMapLevels, glm::max(1U, static_cast<unsigned int>(glm::log2(static_cast<float>(maxSize))) + 1));
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexStorage3D, id.textureType, mipMapLevels, descriptor.internalFormat, width, height, depth);
        if (data) {
            OGL_CALL(glTexSubImage3D, id.textureType, 0, 0, 0, 0, width, height, depth,
                descriptor.format, descriptor.type, data);
        }
        OGL_CALL(glBindTexture, id.textureType, 0);
        InitSampling();
    }

    /**
     * Manages a pre-created texture.
     * @param texID the texture id
     * @param texType the textures type
     */
    GLTexture::GLTexture(TextureRAII texID, GLenum texType, const TextureDescriptor& desc) :
        id{ std::move(texID), texType },
        descriptor(desc),
        width(0),
        height(0),
        depth(0),
        mipMapLevels(1)
    {
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        GLint qResult;
        auto queryType = id.textureType;
        if (id.textureType == GL_TEXTURE_CUBE_MAP) queryType = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        OGL_CALL(glGetTexLevelParameteriv, queryType, 0, GL_TEXTURE_WIDTH, &qResult);
        width = static_cast<unsigned int>(qResult);
        OGL_CALL(glGetTexLevelParameteriv, queryType, 0, GL_TEXTURE_HEIGHT, &qResult);
        height = static_cast<unsigned int>(qResult);
        OGL_CALL(glGetTexLevelParameteriv, queryType, 0, GL_TEXTURE_DEPTH, &qResult);
        depth = static_cast<unsigned int>(qResult);
        InitSampling();
    }

    /** Destructor. */
    GLTexture::~GLTexture() = default;

    /** Initializes the sampler. */
    void GLTexture::InitSampling() const
    {
        SampleLinear();
        SampleWrapClamp();
    }

    /**
     * Activate the texture for rendering
     * @param textureUnit the texture unit to set the texture to
     */
    void GLTexture::ActivateTexture(GLenum textureUnit) const
    {
        OGL_CALL(glActiveTexture, textureUnit);
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
    }

    /**
     *  Activate the texture as an image.
     *  @param imageUnitIndex the index of the image unit to activate the texture for
     *  @param mipLevel the MipMap level to bind
     *  @param accessType the access type needed
     */
    void GLTexture::ActivateImage(GLuint imageUnitIndex, GLint mipLevel, GLenum accessType) const
    {
        OGL_CALL(glBindImageTexture, imageUnitIndex, id.textureId, mipLevel, GL_TRUE, 0, accessType, descriptor.internalFormat);
    }

    /**
     * Add a 2d bitmap to a texture array.
     * @param file the file to add
     * @param slice the array slice to add the files content to
     */
    void GLTexture::AddTextureToArray(const std::string& file, unsigned int slice) const
    {
        stbi_set_flip_vertically_on_load(1);
        auto channelsNeeded = 0;
        switch (descriptor.format) {
        case GL_RED: channelsNeeded = 1; break;
        case GL_RG: channelsNeeded = 2; break;
        case GL_RGB: channelsNeeded = 3; break;
        case GL_RGBA: channelsNeeded = 4; break;
        default:
            LOG(ERROR) << L"Unknown texture format (" << descriptor.format << ").";
            throw std::runtime_error("Unknown texture format.");
        }

        auto imgWidth = 0, imgHeight = 0, imgChannels = 0;
        auto image = stbi_load(file.c_str(), &imgWidth, &imgHeight, &imgChannels, channelsNeeded);
        if (!image) {
            LOG(ERROR) << L"Could not load texture \"" << file.c_str() << L"\".";
            throw std::runtime_error("Could not load texture \"" + file + "\".");
        }

        if (width != imgWidth || height != imgHeight) {
            LOG(ERROR) << L"Texture \"" << file.c_str() << L"\" has the wrong format!";
            stbi_image_free(image);
            throw std::runtime_error("Texture \"" + file + "\" has the wrong format.");
        }

        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexSubImage3D, id.textureType, 0, 0, 0, slice, width, height, 1,
            descriptor.format, descriptor.type, image);
        OGL_CALL(glBindTexture, id.textureType, 0);

        stbi_image_free(image);
    }

    /**
     *  Sets the textures data.
     *  @param data the data to set.
     */
    void GLTexture::SetData(const void* data) const
    {
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        switch (id.textureType)
        {
        case GL_TEXTURE_1D:
            OGL_CALL(glTexSubImage1D, id.textureType, 0, 0, width, descriptor.format, descriptor.type, data);
            break;
        case GL_TEXTURE_2D:
            OGL_CALL(glTexSubImage2D, id.textureType, 0, 0, 0, width, height, descriptor.format, descriptor.type, data);
            break;
        case GL_TEXTURE_3D:
            OGL_CALL(glTexSubImage3D, id.textureType, 0, 0, 0, 0, width, height, depth, descriptor.format, descriptor.type, data);
            break;
        default:
            throw std::runtime_error("Texture format not supported for upload.");
        }        
        OGL_CALL(glBindTexture, id.textureType, 0);
    }

    /**
     *  Downloads the textures data to a vector.
     *  @param data the vector to contain the data.
     */
    void GLTexture::DownloadData(std::vector<uint8_t>& data) const
    {
        data.resize(width * height * depth * descriptor.bytesPP);
        assert(data.size() != 0);

        // TODO: create external PBOs for real asynchronous up-/download [8/19/2015 Sebastian Maisch]
        BufferRAII pbo;
        OGL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, pbo);
        OGL_CALL(glBufferData, GL_PIXEL_PACK_BUFFER, data.size(), nullptr, GL_STREAM_READ);

        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glGetTexImage, id.textureType, 0, descriptor.format, descriptor.type, 0);

        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        auto gpuMem = OGL_CALL(glMapBuffer, GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if (gpuMem) {
            memcpy(data.data(), gpuMem, data.size());
            OGL_CALL(glUnmapBuffer, GL_PIXEL_PACK_BUFFER);
        }

        OGL_CALL(glBindTexture, id.textureType, 0);
        OGL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, 0);
    }

    void GLTexture::DownloadData8Bit(std::vector<uint8_t>& data) const
    {
        auto comp = 0;
        if (descriptor.format == GL_RED) comp = 1;
        else if (descriptor.format == GL_RG) comp = 2;
        else if (descriptor.format == GL_RGB) comp = 3;
        else if (descriptor.format == GL_RGBA) comp = 4;
        else {
            LOG(WARNING) << "Texture format not supported for downloading.";
            return;
        }

        data.resize(width * height * depth * comp);
        assert(data.size() != 0);

        // TODO: create external PBOs for real asynchronous up-/download [8/19/2015 Sebastian Maisch]
        BufferRAII pbo;
        OGL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, pbo);
        OGL_CALL(glBufferData, GL_PIXEL_PACK_BUFFER, data.size(), nullptr, GL_STREAM_READ);

        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glGetTexImage, id.textureType, 0, descriptor.format, GL_UNSIGNED_BYTE, 0);

        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        auto gpuMem = OGL_CALL(glMapBuffer, GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if (gpuMem) {
            memcpy(data.data(), gpuMem, data.size());
            OGL_CALL(glUnmapBuffer, GL_PIXEL_PACK_BUFFER);
        }

        OGL_CALL(glBindTexture, id.textureType, 0);
        OGL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, 0);
    }

    void GLTexture::SaveTextureToFile(const std::string& filename) const
    {
        auto comp = 0;
        if (descriptor.format == GL_RED) comp = 1;
        else if(descriptor.format == GL_RG) comp = 2;
        else if(descriptor.format == GL_RGB) comp = 3;
        else if(descriptor.format == GL_RGBA) comp = 4;
        else {
            LOG(WARNING) << "Texture format not supported for saving.";
            return;
        }
        std::vector<uint8_t> screenData;
        DownloadData8Bit(screenData);

        auto stride = width * 4;
        for (unsigned i = 0; i < height / 2; ++i) {
            auto first = i * stride;
            auto last = ((i + 1) * stride) - 1;
            std::swap_ranges(screenData.begin() + first, screenData.begin() + last, screenData.end() - last - 1);
        }
        stbi_write_png(filename.c_str(), width, height, comp, screenData.data(), stride * sizeof(uint8_t));
    }

    /**
     *  Uploads data to the texture from a vector.
     *  @param data the vector that contains the data.
     */
    void GLTexture::UploadData(std::vector<uint8_t>& data) const
    {
        assert(data.size() != 0);

        // TODO: create external PBOs for real asynchronous up-/download [8/19/2015 Sebastian Maisch]
        BufferRAII pbo;
        OGL_CALL(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, pbo);
        OGL_CALL(glBufferData, GL_PIXEL_UNPACK_BUFFER, data.size(), nullptr, GL_STREAM_DRAW);

        auto gpuMem = OGL_CALL(glMapBuffer, GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (gpuMem) {
            memcpy(gpuMem, data.data(), data.size());
            OGL_CALL(glUnmapBuffer, GL_PIXEL_UNPACK_BUFFER);
        }

        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        if (id.textureType == GL_TEXTURE_3D || id.textureType == GL_TEXTURE_2D_ARRAY) {
            OGL_CALL(glTexSubImage3D, id.textureType, 0, 0, 0, 0, width, height, depth, descriptor.format, descriptor.type, 0);
        } else if (id.textureType == GL_TEXTURE_2D || id.textureType == GL_TEXTURE_1D_ARRAY) {
            OGL_CALL(glTexSubImage2D, id.textureType, 0, 0, 0, width, height, descriptor.format, descriptor.type, 0);
        } else {
            OGL_CALL(glTexSubImage1D, id.textureType, 0, 0, width, descriptor.format, descriptor.type, 0);
        }

        OGL_CALL(glBindTexture, id.textureType, 0);
        OGL_CALL(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, 0);
    }

    /**
     *  Generates MipMaps for the texture.
     */
    void GLTexture::GenerateMipMaps() const
    {
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glGenerateMipmap, id.textureType);
        OGL_CALL(glBindTexture, id.textureType, 0);
    }

    /**
     *  Generates min/max maps for the texture (only 4 components allowed, r: avg, g: min, b: max).
     *  @param minMaxProg the program for generating the Min/Max MipMaps
     *  @param uniformNames the names of the uniforms of the images used in the program
     */
    void GLTexture::GenerateMinMaxMaps(GPUProgram* minMaxProg, const std::vector<BindingLocation>& uniformNames) const
    {
        assert(descriptor.format == GL_RGBA || descriptor.format == GL_RGBA_INTEGER);

        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glGenerateMipmap, id.textureType);

        // unsigned int max_res = glm::max(width, glm::max(height, depth));
        auto firstMipRes = glm::ivec3(width, height, depth);
        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(firstMipRes) / 8.0f));

        minMaxProg->UseProgram();
        minMaxProg->SetUniform(uniformNames[0], 0);
        minMaxProg->SetUniform(uniformNames[1], 1);
        for (unsigned int i = 1; i < mipMapLevels; ++i) {
            numGroups = glm::ivec3(glm::ceil(glm::vec3(numGroups) * 0.5f));
            ActivateImage(0, i - 1, GL_READ_ONLY);
            ActivateImage(1, i, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);
        }

        InitSampling();
    }

    /**
     *  Clears a texture.
     *  @param mipLevel the MipMap level to clear.
     *  @param data the clear color.
     */
    void GLTexture::ClearTexture(unsigned int mipLevel, const glm::vec4& data) const
    {
        assert(mipLevel < mipMapLevels);
        OGL_CALL(glClearTexImage, id.textureId, mipLevel, descriptor.format, GL_FLOAT, &data);
    }

    /**
     *  Returns the dimensions of a mip map level.
     */
    glm::uvec3 GLTexture::GetLevelDimensions(int level) const
    {
        GLint w, h, d;
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glGetTexLevelParameteriv, id.textureType, level, GL_TEXTURE_WIDTH, &w);
        OGL_CALL(glGetTexLevelParameteriv, id.textureType, level, GL_TEXTURE_HEIGHT, &h);
        OGL_CALL(glGetTexLevelParameteriv, id.textureType, level, GL_TEXTURE_DEPTH, &d);
        OGL_CALL(glBindTexture, id.textureType, 0);
        return glm::uvec3(static_cast<unsigned int>(w), static_cast<unsigned int>(h), static_cast<unsigned int>(d));
    }

    /**
     *  Sets the sampler parameters for mirroring.
     */
    void GLTexture::SampleWrapMirror() const
    {
        SetSampleWrap(GL_MIRRORED_REPEAT);
    }

    /**
     *  Sets the sampler parameters for clamping.
     */
    void GLTexture::SampleWrapClamp() const
    {
        SetSampleWrap(GL_CLAMP_TO_EDGE);
    }

    void GLTexture::SampleWrapRepeat() const
    {
        SetSampleWrap(GL_REPEAT);
    }

    void GLTexture::SampleWrapMirrorClamp() const
    {
        SetSampleWrap(GL_MIRROR_CLAMP_TO_EDGE);
    }

    void GLTexture::SampleWrapBorderColor(const glm::vec4& color) const
    {
        if (id.textureType == GL_TEXTURE_2D_MULTISAMPLE) return;
        SetSampleWrap(GL_CLAMP_TO_BORDER);
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexParameterfv, id.textureType, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
        OGL_CALL(glBindTexture, id.textureType, 0);
    }

    /**
     *  Sets the sampler parameters for texture wrapping mode.
     *  @param param the wrapping parameter
     */
    void GLTexture::SetSampleWrap(GLint param) const
    {
        if (id.textureType == GL_TEXTURE_2D_MULTISAMPLE) return;
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_WRAP_S, param);
        if (id.textureType == GL_TEXTURE_2D || id.textureType == GL_TEXTURE_3D) {
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_WRAP_T, param);
        }
        if (id.textureType == GL_TEXTURE_3D) {
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_WRAP_R, param);
        }
        OGL_CALL(glBindTexture, id.textureType, 0);
    }

    /**
     *  Sets the sampler parameters for linear filtering.
     */
    void GLTexture::SampleLinear() const
    {
        if (id.textureType == GL_TEXTURE_2D_MULTISAMPLE) return;
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        if (mipMapLevels > 1) {
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_BASE_LEVEL, 0);
        } else {
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        OGL_CALL(glBindTexture, id.textureType, 0);
    }

    /**
     *  Sets the sampler parameters for box filtering.
     */
    void GLTexture::SampleNearest() const
    {
        if (id.textureType == GL_TEXTURE_2D_MULTISAMPLE) return;
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        if (mipMapLevels > 1) {
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_BASE_LEVEL, 0);
        } else {
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        OGL_CALL(glBindTexture, id.textureType, 0);
    }

    void GLTexture::ActivateShadowMapComparison() const
    {
        if (id.textureType == GL_TEXTURE_2D_MULTISAMPLE) return;
        OGL_CALL(glBindTexture, id.textureType, id.textureId);
        OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        OGL_CALL(glTexParameteri, id.textureType, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        OGL_CALL(glBindTexture, id.textureType, 0);
    }
}
