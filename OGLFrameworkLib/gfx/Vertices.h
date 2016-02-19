/**
 * @file   Vertices.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.13
 *
 * @brief  Contains all vertices used for rendering.
 */

#ifndef VERTICES_H
#define VERTICES_H

#include "../main.h"

namespace cgu {

    template<int POS_DIM> struct UberMeshPos
    {
        bool posEql(const UberMeshPos&) const { return true; }
    };

    template<> struct UberMeshPos<2>
    {
        glm::vec2 pos;
        bool posEql(const UberMeshPos& rhs) const { return pos == rhs.pos; }
    };

    template<> struct UberMeshPos<3>
    {
        glm::vec3 pos;
        bool posEql(const UberMeshPos& rhs) const { return pos == rhs.pos; }
    };


    template<bool NORMAL> struct UberMeshNormal
    {
        bool normalEql(const UberMeshNormal&) const { return true; }
    };

    template<> struct UberMeshNormal<true>
    {
        glm::vec3 normal;
        bool normalEql(const UberMeshNormal& rhs) const { return normal == rhs.normal; }
    };


    template<bool TEXCOORDS, int TEXCOORD_DIM, int NUM_TEXCOORDS> struct UberMeshTexCoords
    {
        bool texEql(const UberMeshTexCoords&) const { return true; }
    };

    template<int NUM_TEXCOORDS> struct UberMeshTexCoords<true, 2, NUM_TEXCOORDS>
    {
        std::array<glm::vec2, NUM_TEXCOORDS> tex;
        bool texEql(const UberMeshTexCoords& rhs) const { return tex == rhs.tex; }
    };

    template<int NUM_TEXCOORDS> struct UberMeshTexCoords<true, 3, NUM_TEXCOORDS>
    {
        std::array<glm::vec3, NUM_TEXCOORDS> tex;
        bool texEql(const UberMeshTexCoords& rhs) const { return tex == rhs.tex; }
    };


    template<bool TANGENTSPACE> struct UberMeshTangentSpace
    {
        bool tangentEql(const UberMeshTangentSpace&) const { return true; }
    };

    template<> struct UberMeshTangentSpace<true>
    {
        glm::vec3 tangent;
        glm::vec3 binormal;
        bool tangentEql(const UberMeshTangentSpace& rhs) const { return tangent == rhs.tangent && binormal == rhs.binormal; }
    };


    template<bool COLORS, int NUM_COLS> struct UberMeshColors
    {
        bool colEql(const UberMeshColors&) const { return true; }
    };

    template<int NUM_COLS> struct UberMeshColors<true, NUM_COLS>
    {
        std::array<glm::vec4, NUM_COLS> color;
        bool colEql(const UberMeshColors& rhs) const { return color == rhs.color; }
    };


    template<bool IDX, int NUM_IDX> struct UberMeshIndices
    {
        bool idxEql(const UberMeshIndices&) const { return true; }
    };

    template<int NUM_IDX> struct UberMeshIndices<true, NUM_IDX>
    {
        std::array<unsigned int, NUM_IDX> idx;
        bool idxEql(const UberMeshIndices& rhs) const { return idx == rhs.idx; }
    };

    template<int POS_DIM = 3, bool NORMAL = true, int TEXCOORD_DIM = 2, int NUM_TEXCOORDS = 1, bool TANGENTSPACE = false, int NUM_COLS = 0, int NUM_IDX = 0>
    struct UberMeshVertex : public UberMeshPos<POS_DIM>, UberMeshNormal<NORMAL>,
        UberMeshTexCoords<(NUM_TEXCOORDS > 0), TEXCOORD_DIM, NUM_TEXCOORDS>, UberMeshTangentSpace<TANGENTSPACE>,
        UberMeshColors<(NUM_COLS > 0), NUM_COLS>, UberMeshIndices<(NUM_IDX > 0), NUM_IDX>

    {
        static const int POSITION_DIMENSION = POS_DIM;
        static const bool HAS_NORMAL = NORMAL;
        static const bool HAS_TANGENTSPACE = TANGENTSPACE;
        static const int TEXCOORD_DIMENSION = TEXCOORD_DIM;
        static const int NUM_TEXTURECOORDS = NUM_TEXCOORDS;
        static const int NUM_COLORS = NUM_COLS;
        static const int NUM_INDICES = NUM_IDX;

        /**
         * Comparison operator for equality.
         * @param rhs the other LineVertex to compare to
         * @return whether the vertices are equal
         */
        bool operator==(const UberMeshVertex& rhs) const
        {
            return posEql(rhs) && normalEql(rhs) && texEql(rhs) && tangentEql(rhs) && colEql(rhs) && idxEql(rhs);
        }
    };

    using FaceVertex = UberMeshVertex<3, true, 2, 1, false, 0, 0>;
    using LineVertex = UberMeshVertex<3, false, 2, 1, false, 0, 0>;
    using FontVertex = UberMeshVertex<3, false, 2, 0, false, 0, 1>;
    using GUIVertex = UberMeshVertex<3, false, 2, 1, false, 0, 0>;

    /** Represents a vertex of a line. */
    //struct LineVertex
    //{
    //    /** Holds the vertices position. */
    //    glm::vec3 pos;
    //    /** Holds the vertices texture coordinates. */
    //    glm::vec2 tex;

    //    /**
    //     * Comparison operator for equality.
    //     * @param rhs the other LineVertex to compare to
    //     * @return whether the vertices are equal
    //     */
    //    bool operator==(const LineVertex& rhs) const
    //    {
    //        return pos == rhs.pos && tex == rhs.tex;
    //    }
    //};

    /** Represents a vertex of a face. */
    //struct FaceVertex
    //{
    //    /** holds the vertices position. */
    //    glm::vec3 pos;
    //    /** holds the vertices texture coordinates. */
    //    glm::vec2 tex;
    //    /** holds the vertices normal. */
    //    glm::vec3 normal;

    //    /**
    //     * comparison operator for equality.
    //     * @param rhs the other facevertex to compare to
    //     * @return whether the vertices are equal
    //     */
    //    bool operator==(const FaceVertex& rhs) const
    //    {
    //        return pos == rhs.pos && tex == rhs.tex && normal == rhs.normal;
    //    }
    //};

    /** Represents a vertex of a text character. */
    //struct FontVertex
    //{
    //    /** Holds the characters position. */
    //    glm::vec3 pos;
    //    /** Holds the character index to render. */
    //    unsigned int idx;

    //    /**
    //     * Comparison operator for equality.
    //     * @param rhs the other FontVertex to compare to
    //     * @return whether the vertices are equal
    //     */
    //    bool operator==(const FontVertex& rhs) const
    //    {
    //        return pos == rhs.pos && idx == rhs.idx;
    //    }
    //};

    /** Represents a vertex of a GUI element. */
    //struct GUIVertex
    //{
    //    /** Holds the vertices position. */
    //    glm::vec3 pos;
    //    /** Holds the vertices texture coordinates. */
    //    glm::vec2 texCoords;

    //    /**
    //     * Comparison operator for equality.
    //     * @param rhs the other GUIVertex to compare to
    //     * @return whether the vertices are equal
    //     */
    //    bool operator==(const GUIVertex& rhs) const
    //    {
    //        return pos == rhs.pos && texCoords == rhs.texCoords;
    //    }
    //};
}

#endif /* VERTICES_H */
