#pragma once
#include <array>

template <typename T>
struct TypeID
{

    static constexpr std::string_view 
    Get()
    {

#       if defined(_MSC_VER) && !defined(__clang__)
            std::string_view raw_name = __FUNCSIG__;
            std::string_view search_for = "TypeID<";
            size_t start = raw_name.find(search_for) + search_for.length();
            size_t end = raw_name.find(">::Get(void)");
            return raw_name.substr(start, end - start);
#       elif defined(__clang__)
            std::string_view raw_name = __PRETTY_FUNCTION__;
            std::string_view search_for = "T = ";
            size_t start = raw_name.find(search_for) + search_for.length();
            size_t end = raw_name.find("]", start);
            return raw_name.substr(start, end - start);
#       elif defined(__GNUC__)
            std::string_view raw_name = __PRETTY_FUNCTION__;
            std::string_view search_for = "T = ";
            size_t start = raw_name.find(search_for) + search_for.length();
            size_t end = raw_name.find(";", start);
            return raw_name.substr(start, end - start);
#       else
#           pragma error "Unsupported compiler for TypeID utility."
#       endif

    }

    static constexpr size_t
    GetHash()
    {
        
        // FNV-1A Hashing Algorithm
        constexpr std::string_view type_name = TypeID<T>::Get();
        constexpr size_t type_length = type_name.length();
        constexpr const char *string = type_name.data();

        uint64_t hash = 14695981039346656037ull;
        for (std::size_t i = 0; i < type_length; ++i)
        {
            hash = (hash ^ static_cast<uint64_t>(string[i])) * 1099511628211ull;
        }

        return hash;

    }

    static inline constexpr std::string_view Value = TypeID<T>::Get();
    static inline constexpr size_t Hash = TypeID<T>::GetHash();

};

template <typename... Args>
struct TypeIDArray
{
    
    static constexpr std::array<std::string_view, sizeof...(Args)>
    Get()
    {
        return { TypeID<Args>::Get()... };
    }

    static constexpr std::array<size_t, sizeof...(Args)>
    GetHashes()
    {
        return { TypeID<Args>::GetHash()... };
    }

    static inline constexpr auto Values = TypeIDArray::Get();
    static inline constexpr auto Hashes = TypeIDArray::GetHashes();
    
};
