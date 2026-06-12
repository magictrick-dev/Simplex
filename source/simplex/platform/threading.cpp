#pragma once
#include <simplex/platform/threading.hpp>
#include <simplex/static_string.hpp>

static thread_local spx::static_string<char, 256> thread_name { "Unnamed" };

spx::string_view<char> spx::mt::this_thread::
get_name()
{
    return thread_name;
}

void spx::mt::this_thread::
set_name(spx::string_view<char> name)
{
    thread_name.clear();
    thread_name.append(name.begin(), name.length());
}