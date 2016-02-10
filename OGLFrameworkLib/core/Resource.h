/**
 * @file   Resource.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.31
 *
 * @brief  Contains Resource, a base class for all managed resources.
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "main.h"
#include <boost/lexical_cast.hpp>

namespace cgu {

    class ApplicationBase;

    /**
     * @brief  Base class for all managed resources.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2013.12.31
     */
    class Resource
    {
    public:
        Resource(const std::string& resourceId, ApplicationBase* app);
        Resource(const Resource&);
        Resource& operator=(const Resource&);
        Resource(Resource&&);
        Resource& operator=(Resource&&);
        virtual ~Resource();

        const std::string& getId() const;
        const std::string& GetFilename() const { return parameters[0]; }

    protected:
        /** A list of sub-resources. */
        using SubResourceList = std::vector<std::string>;
        /** A list of parameters. */
        using ParameterList = std::vector<std::string>;
        /**  A map of flag and value parameters. */
        using ParameterMap = std::unordered_map<std::string, std::string>;

        std::string FindResourceLocation(const std::string& localFilename) const;
        const ParameterList& GetParameters() const { return parameters; };
        const SubResourceList& GetSubresourceIds() const { return subresourceIds; };
        std::string GetParameter(unsigned int index) const { return parameters[index]; };
        std::string GetNamedParameterString(const std::string& name) const;
        template<typename T> T GetNamedParameterValue(const std::string& name, const T& def) const { 
            auto resultString = GetNamedParameterString(name);
            T result = def;
            if (resultString.size() != 0) result = boost::lexical_cast<unsigned int>(resultString);
            return result;
        };
        bool CheckNamedParameterFlag(const std::string& name) const;


        /** Holds the application object for dependencies. */
        ApplicationBase* application;

    private:
        static std::string GetNormalizedResourceId(const std::string& resId);

        /** Holds the resources id. */
        std::string id;
        /** Holds the sub-resources ids. */
        SubResourceList subresourceIds;
        /** Holds the parameters. */
        ParameterList parameters;
        /** Holds the named parameters with (optional) values. */
        ParameterMap namedParameters;
    };
}

#endif /* RESOURCE_H */
