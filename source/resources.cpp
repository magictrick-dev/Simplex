#include <utils/resources.hpp>

bool ResourceManager::
resource_by_path_exists(const std::filesystem::path &path)
{

    const std::filesystem::path canon_path = std::filesystem::weakly_canonical(path);
    const bool result = (resource_path_map.find(canon_path) != resource_path_map.end());
    return result;

}

bool ResourceManager::
resource_by_alias_exists(const std::string &alias)
{

    const bool result = (resource_alias_map.find(alias) != resource_alias_map.end());
    return result;

}

ResourceManagerResult ResourceManager:: 
unload_all_resources()
{
    SIMPLEX_NO_IMPLEMENTATION("Resource Manager unloading not required yet.");
    return ResourceManagerResult_OK;
}

ResourceManagerResult ResourceManager:: 
unload_resource_by_path(const std::filesystem::path &path)
{

    return ResourceManagerResult_OK;
}

ResourceManagerResult ResourceManager:: 
unload_resource_by_alias(const std::string &alias)
{

    return ResourceManagerResult_OK;
}
