#include <package/pack.hpp>
#include <package/descriptor.hpp>
#include <vendor/lz4/lz4.h>
#include <vendor/nlohmann/json.hpp>

using namespace spx::package;

packer_v1::
packer_v1()
{

}

packer_v1::
~packer_v1()
{

}

bool packer_v1::
add_resource(spx::string_view<char> resource_path)
{

}

bool packer_v1::
add_resources_from_manifest(spx::string_view<char> manifest_path)
{

}

bool packer_v1::
verify_path(spx::string_view<char> path)
{

}