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
        void SetPosition(float p, int dim) {}
        static void posAttribName(std::vector<std::string>&) { }
        template<class VTX> static void posAttribBind(GLVertexAttributeArray*, const std::vector<BindingLocation>&, int&) { }
    };

    template<> struct UberMeshPos<2>
    {
        glm::vec2 pos;
        bool posEql(const UberMeshPos& rhs) const { return pos == rhs.pos; }
        void SetPosition(float p, int dim) { pos[dim] = p; }
        static void posAttribName(std::vector<std::string>& attribNames) { attribNames.push_back("position"); }
        template<class VTX>
        static void posAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };

    template<> struct UberMeshPos<3>
    {
        glm::vec3 pos;
        bool posEql(const UberMeshPos& rhs) const { return pos == rhs.pos; }
        void SetPosition(float p, int dim) { pos[dim] = p; }
        static void posAttribName(std::vector<std::string>& attribNames) { attribNames.push_back("position"); }
        template<class VTX>
        static void posAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };

    template<bool NORMAL> struct UberMeshNormal
    {
        bool normalEql(const UberMeshNormal&) const { return true; }
        void SetNormal(const glm::vec3& n) { }
        static void normalAttribName(std::vector<std::string>&) { }
        template<class VTX> static void normalAttribBind(GLVertexAttributeArray*, const std::vector<BindingLocation>&, int&) { }
    };

    template<> struct UberMeshNormal<true>
    {
        glm::vec3 normal;
        bool normalEql(const UberMeshNormal& rhs) const { return normal == rhs.normal; }
        void SetNormal(const glm::vec3& n) { normal = n; }
        static void normalAttribName(std::vector<std::string>& attribNames) { attribNames.push_back("normal"); }
        template<class VTX>
        static void normalAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };


    template<bool TEXCOORDS, int TEXCOORD_DIM, int NUM_TEXCOORDS> struct UberMeshTexCoords
    {
        bool texEql(const UberMeshTexCoords&) const { return true; }
        void SetTexCoord(float v, int i, int dim) {}
        static void texAttribName(std::vector<std::string>&) { }
        template<class VTX> static void texAttribBind(GLVertexAttributeArray*, const std::vector<BindingLocation>&, int&) { }
    };

    template<int NUM_TEXCOORDS> struct UberMeshTexCoords<true, 2, NUM_TEXCOORDS>
    {
        std::array<glm::vec2, NUM_TEXCOORDS> tex;
        bool texEql(const UberMeshTexCoords& rhs) const { return tex == rhs.tex; }
        void SetTexCoord(float v, int i, int dim) { tex[i][dim] = v; }
        static void texAttribName(std::vector<std::string>& attribNames);
        template<class VTX>
        static void texAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };

    template<int NUM_TEXCOORDS> struct UberMeshTexCoords<true, 3, NUM_TEXCOORDS>
    {
        std::array<glm::vec3, NUM_TEXCOORDS> tex;
        bool texEql(const UberMeshTexCoords& rhs) const { return tex == rhs.tex; }
        void SetTexCoord(float v, int i, int dim) { tex[i][dim] = v; }
        static void texAttribName(std::vector<std::string>& attribNames);
        template<class VTX>
        static void texAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };


    template<bool TANGENTSPACE> struct UberMeshTangentSpace
    {
        bool tangentEql(const UberMeshTangentSpace&) const { return true; }
        void SetTangent(const glm::vec3& t) { }
        void SetBinormal(const glm::vec3& b) { }
        static void tangentAttribName(std::vector<std::string>&) { }
        template<class VTX> static void tangentAttribBind(GLVertexAttributeArray*, const std::vector<BindingLocation>&, int&) { }
    };

    template<> struct UberMeshTangentSpace<true>
    {
        glm::vec3 tangent;
        glm::vec3 binormal;
        bool tangentEql(const UberMeshTangentSpace& rhs) const { return tangent == rhs.tangent && binormal == rhs.binormal; }
        void SetTangent(const glm::vec3& t) { tangent = t; }
        void SetBinormal(const glm::vec3& b) { binormal = b; }
        static void tangentAttribName(std::vector<std::string>& attribNames) { attribNames.push_back("tangent"); attribNames.push_back("binormal"); }
        template<class VTX>
        static void tangentAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };


    template<bool COLORS, int NUM_COLS> struct UberMeshColors
    {
        bool colEql(const UberMeshColors&) const { return true; }
        void SetColor(const glm::vec4& c, int i) {}
        static void colAttribName(std::vector<std::string>&) { }
        template<class VTX> static void colAttribBind(GLVertexAttributeArray*, const std::vector<BindingLocation>&, int&) { }
    };

    template<int NUM_COLS> struct UberMeshColors<true, NUM_COLS>
    {
        std::array<glm::vec4, NUM_COLS> color;
        bool colEql(const UberMeshColors& rhs) const { return color == rhs.color; }
        void SetColor(const glm::vec4& c, int i) { color[i] = c; }
        static void colAttribName(std::vector<std::string>& attribNames);
        template<class VTX>
        static void colAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };


    template<bool IDX, int NUM_IDX> struct UberMeshIndices
    {
        bool idxEql(const UberMeshIndices&) const { return true; }
        static void idxAttribName(std::vector<std::string>&) { }
        template<class VTX> static void idxAttribBind(GLVertexAttributeArray*, const std::vector<BindingLocation>&, int&) { }
    };

    template<int NUM_IDX> struct UberMeshIndices<true, NUM_IDX>
    {
        std::array<unsigned int, NUM_IDX> idx;
        bool idxEql(const UberMeshIndices& rhs) const { return idx == rhs.idx; }
        static void idxAttribName(std::vector<std::string>& attribNames);
        template<class VTX>
        static void idxAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt);
    };

    template<int POS_DIM = 3, bool NORMAL = true, int TEXCOORD_DIM = 2, int NUM_TEXCOORDS = 1, bool TANGENTSPACE = false, int NUM_COLS = 0, int NUM_IDX = 0>
    struct UberMeshVertex : public UberMeshPos<POS_DIM>, UberMeshNormal<NORMAL>,
        UberMeshTexCoords<(NUM_TEXCOORDS > 0), TEXCOORD_DIM, NUM_TEXCOORDS>, UberMeshTangentSpace<TANGENTSPACE>,
        UberMeshColors<(NUM_COLS > 0), NUM_COLS>, UberMeshIndices<(NUM_IDX > 0), NUM_IDX>

    {
        using VertexType = UberMeshVertex<POS_DIM, NORMAL, TEXCOORD_DIM, NUM_TEXCOORDS, TANGENTSPACE, NUM_COLS, NUM_IDX>;
        using PosType = UberMeshPos<POS_DIM>;
        using NormalType = UberMeshNormal<NORMAL>;
        using TexType = UberMeshTexCoords<(NUM_TEXCOORDS > 0), TEXCOORD_DIM, NUM_TEXCOORDS>;
        using TangentType = UberMeshTangentSpace<TANGENTSPACE>;
        using ColorType = UberMeshColors<(NUM_COLS > 0), NUM_COLS>;
        using IndexType = UberMeshIndices<(NUM_IDX > 0), NUM_IDX>;
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

        static void GatherAttributeNames(std::vector<std::string>& attribNames)
        {
            PosType::posAttribName(attribNames);
            NormalType::normalAttribName(attribNames);
            TexType::texAttribName(attribNames);
            TangentType::tangentAttribName(attribNames);
            ColorType::colAttribName(attribNames);
            IndexType::idxAttribName(attribNames);
        }

        static void VertexAttributeSetup(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions)
        {

            vao->StartAttributeSetup();
            auto cPos = 0;
            PosType::template posAttribBind<VertexType>(vao, shaderPositions, cPos);
            NormalType::template normalAttribBind<VertexType>(vao, shaderPositions, cPos);
            TexType::template texAttribBind<VertexType>(vao, shaderPositions, cPos);
            TangentType::template tangentAttribBind<VertexType>(vao, shaderPositions, cPos);
            ColorType::template colAttribBind<VertexType>(vao, shaderPositions, cPos);
            IndexType::template idxAttribBind<VertexType>(vao, shaderPositions, cPos);
            vao->EndAttributeSetup();
        }
    };

    template <class VTX>
    void UberMeshPos<2>::posAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        if (shaderPositions[acnt]->iBinding >= 0) vao->AddVertexAttribute(shaderPositions[acnt], 2, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, pos));
        acnt += 1;
    }

    template <class VTX>
    void UberMeshPos<3>::posAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        if (shaderPositions[acnt]->iBinding >= 0) vao->AddVertexAttribute(shaderPositions[acnt], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, pos));
        acnt += 1;
    }

    template <class VTX>
    void UberMeshNormal<true>::normalAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        if (shaderPositions[acnt]->iBinding >= 0) vao->AddVertexAttribute(shaderPositions[acnt], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, normal));
        acnt += 1;
    }

    template <int NUM_TEXCOORDS>
    void UberMeshTexCoords<true, 2, NUM_TEXCOORDS>::texAttribName(std::vector<std::string>& attribNames)
    {
        std::stringstream attributeNameStr;
        for (auto i = 0; i < NUM_TEXCOORDS; ++i) { attributeNameStr.clear(); attributeNameStr << "tex[" << i << "]"; attribNames.push_back(attributeNameStr.str()); }
    }

    template <int NUM_TEXCOORDS>
    template <class VTX>
    void UberMeshTexCoords<true, 2, NUM_TEXCOORDS>::texAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        for (auto i = 0; i < NUM_TEXCOORDS; ++i) {
            if (shaderPositions[acnt]->iBinding >= 0) vao->AddVertexAttribute(shaderPositions[acnt], 2, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, tex[i]));
            acnt += 1;
        }
    }

    template <int NUM_TEXCOORDS>
    void UberMeshTexCoords<true, 3, NUM_TEXCOORDS>::texAttribName(std::vector<std::string>& attribNames)
    {
        std::stringstream attributeNameStr;
        for (auto i = 0; i < NUM_TEXCOORDS; ++i) { attributeNameStr.clear(); attributeNameStr << "tex[" << i << "]"; attribNames.push_back(attributeNameStr.str()); }
    }

    template <int NUM_TEXCOORDS>
    template <class VTX>
    void UberMeshTexCoords<true, 3, NUM_TEXCOORDS>::texAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        for (auto i = 0; i < NUM_TEXCOORDS; ++i) {
            if (shaderPositions[acnt]->iBinding >= 0) vao->AddVertexAttribute(shaderPositions[acnt], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, tex[i]));
            acnt += 1;
        }
    }

    template <class VTX>
    void UberMeshTangentSpace<true>::tangentAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        if (shaderPositions[acnt]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[acnt], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, tangent));
        }
        acnt += 1;
        if (shaderPositions[acnt]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[acnt], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, binormal));
        }
        acnt += 1;
    }

    template <int NUM_COLS>
    void UberMeshColors<true, NUM_COLS>::colAttribName(std::vector<std::string>& attribNames)
    {
        std::stringstream attributeNameStr;
        for (auto i = 0; i < NUM_COLS; ++i) { attributeNameStr.clear(); attributeNameStr << "color[" << i << "]"; attribNames.push_back(attributeNameStr.str()); }
    }

    template <int NUM_COLS>
    template <class VTX>
    void UberMeshColors<true, NUM_COLS>::colAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        for (auto i = 0; i < NUM_COLS; ++i) {
            if (shaderPositions[acnt]->iBinding >= 0)
                vao->AddVertexAttribute(shaderPositions[acnt], 4, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, color[i]));

            acnt += 1;
        }
    }

    template <int NUM_IDX>
    void UberMeshIndices<true, NUM_IDX>::idxAttribName(std::vector<std::string>& attribNames)
    {
        std::stringstream attributeNameStr;
        for (auto i = 0; i < NUM_IDX; ++i) { attributeNameStr.clear(); attributeNameStr << "index[" << i << "]"; attribNames.push_back(attributeNameStr.str()); }
    }

    template <int NUM_IDX>
    template <class VTX>
    void UberMeshIndices<true, NUM_IDX>::idxAttribBind(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions, int& acnt)
    {
        for (auto i = 0; i < NUM_IDX; ++i) {
            if (shaderPositions[acnt]->iBinding >= 0)
                vao->AddVertexAttribute(shaderPositions[acnt], 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(VTX), offsetof(VTX, idx[i]));

            acnt += 1;
        }
    }

    using FaceVertex = UberMeshVertex<3, true, 2, 1, false, 0, 0>;
    using FaceTangentVertex = UberMeshVertex<3, true, 2, 1, true, 0, 0>;
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
