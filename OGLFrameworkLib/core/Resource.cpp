/**
* @file    Resource.cpp
* @author  Sebastian Maisch <sebastian.maisch@googlemail.com>
* @date    2014.03.29
*
* @brief   Implementations for the resource class.
*/

#include "Resource.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include "app/ApplicationBase.h"
#include "app/Configuration.h"

namespace cgu {
    /**
     * Constructor.
     * @param resourceId the resource id to use
     * @param app the application object for dependencies
     */
    Resource::Resource(const std::string& resourceId, ApplicationBase* app) :
        application(app),
        id(GetNormalizedResourceId(resourceId))
    {
        boost::split(subresourceIds, id, boost::is_any_of("|"));
        for (auto& sr : subresourceIds) boost::trim(sr);

        if (subresourceIds.size() == 1) {
            boost::split(parameters, subresourceIds[0], boost::is_any_of(","));
            for (auto& param : parameters) {
                boost::trim(param);
                const boost::regex nameValueRegex("^-([\\w-]*)=(.*)$");
                const boost::regex flagRegex("^-([\\w-]*)$");
                boost::smatch paramMatch;
                if (boost::regex_match(param, paramMatch, nameValueRegex)) {
                    namedParameters.insert(std::make_pair(paramMatch[1].str(), paramMatch[2].str()));
                } else if (boost::regex_match(param, paramMatch, flagRegex)) {
                    namedParameters.insert(std::make_pair(paramMatch[1].str(), ""));
                }
            }
        }
    };

    /** Default copy constructor. */
    Resource::Resource(const Resource&) = default;

    /** Default assignment operator. */
    Resource& Resource::operator=(const Resource&) = default;

    /** Move constructor. */
    Resource::Resource(Resource&& orig) :
        application(std::move(orig.application)),
        id(std::move(orig.id))
    {
        orig.application = nullptr;
    };

    /** Move assignment operator. */
    Resource& Resource::operator=(Resource&& orig)
    {
        if (this != &orig) {
            this->~Resource();
            id = std::move(orig.id);
            application = orig.application;
            orig.application = nullptr;
        }
        return *this;
    };

    Resource::~Resource() = default;

    /**
    * Accessor to the resources id.
    * @return the resources id
    */
    const std::string& Resource::getId() const
    {
        return id;
    };

    std::string Resource::GetNamedParameterString(const std::string& name) const
    {
        auto it = namedParameters.find(name);
        if (it != namedParameters.end()) return it->second;
        return "";
    }

    bool Resource::CheckNamedParameterFlag(const std::string& name) const
    {
        return namedParameters.find(name) != namedParameters.end();
    }

    /**
         *  Returns the normalized resource id (no global parameters).
         *  @param the resource id.
         *  @return the normalized resource id.
         */
    std::string Resource::GetNormalizedResourceId(const std::string& resId)
    {
        std::vector<std::string> subresourcesAndGlobalParams;
        boost::split_regex(subresourcesAndGlobalParams, resId, boost::regex("\\|,"));
        for (auto& sr : subresourcesAndGlobalParams) boost::trim(sr);

        SubResourceList subresources;
        boost::split(subresources, subresourcesAndGlobalParams[0], boost::is_any_of("|"));
        for (auto& sr : subresources) {
            boost::trim(sr);
            if (subresourcesAndGlobalParams.size() > 1) sr += "," + subresourcesAndGlobalParams[1];
        }
        return boost::join(subresources, "|");
    }

    /**
     *  Returns the actual location of the resource by looking into all search paths.
     *  @param localFilename the file name local to any resource base directory.
     *  @return the path to the resource.
     */
    std::string Resource::FindResourceLocation(const std::string& localFilename) const
    {
        auto filename = application->GetConfig().resourceBase + "/" + localFilename;
        if (boost::filesystem::exists(filename)) return filename;

        for (const auto& dir : application->GetConfig().resourceDirs) {
            filename = dir + "/" + localFilename;
            if (boost::filesystem::exists(filename)) return filename;
        }

        LOG(ERROR) << L"Cannot find local resource file \"" << localFilename.c_str() << L"\".";
        throw resource_loading_error() << ::boost::errinfo_file_name(localFilename) << resid_info(id)
            << errdesc_info("Cannot find local resource file.");
    }
}
