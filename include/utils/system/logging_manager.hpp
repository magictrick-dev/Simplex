#pragma once
#include <utils/defs.hpp>

#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <deque>
#include <array>
#include <algorithm>

struct LoggingMessage
{
    size_t origin;
    size_t issue;
    std::string level;
    std::string classification;
    std::string message;
};

enum class LoggingLevel : size_t
{
    Naked,
    Debug,
    Diagnostic,
    Information,
    Warning,
    Critical,
    Error,
    _Count,
};

enum class LoggingClassification : size_t
{
    Undefined,
    Internal,
    Engine,
    Resource,
    _Count,
};

enum LoggingManagerSwitch
{
    LoggingManagerSwitch_Disable,
    LoggingManagerSwitch_EnableProduction,
    LoggingManagerSwitch_EnableMinimal,
    LoggingManagerSwitch_EnableFull,
};

class LoggingManager
{

    public:
        static inline LoggingManager& Get()
        {
            static LoggingManager manager = {};
            return manager;
        }

        template <LoggingLevel l, LoggingClassification c, typename... Args>
        static inline void DispatchMessage(std::string_view message, Args&&... args)
        {
            
        }

        static inline constexpr std::array<std::string_view, static_cast<size_t>(LoggingLevel::_Count)>
        GetLoggingLevelsArray()
        {

            constexpr std::array levels =
            {
                std::string_view("Naked"),
                std::string_view("Debug"),
                std::string_view("Diagnostic"),
                std::string_view("Information"),
                std::string_view("Warning"),
                std::string_view("Critical"),
                std::string_view("Error"),
            };

            static_assert(
                levels.size() == static_cast<size_t>(LoggingLevel::_Count), 
                "Mismatched levels to the implementation count, make sure to update the list!"
            );

            return levels;

        }

        static inline constexpr std::array<std::string_view, static_cast<size_t>(LoggingClassification::_Count)>
        GetLoggingClassificationsArray()
        {

            constexpr std::array classifications =
            {
                std::string_view("Undefined"),
                std::string_view("Internal"),
                std::string_view("Engine"),
                std::string_view("Resource"),
            };

            static_assert(
                classifications.size() == static_cast<size_t>(LoggingClassification::_Count),
                "Mismatched classifications to the implementation count, make sure to update the list!"
            );

            return classifications;

        }

        static inline constexpr std::string_view
        GetLoggingLevelString(LoggingLevel level)
        {
            auto levels_array = GetLoggingLevelsArray();
            size_t index = static_cast<size_t>(level);
            return levels_array[index];
        }

        static inline constexpr std::string_view
        GetLoggingClassificationString(LoggingClassification classification)
        {
            auto levels_array = GetLoggingClassificationsArray();
            size_t index = static_cast<size_t>(classification);
            return levels_array[index];
        }

        static constexpr inline size_t
        GetLoggingHeaderLength()
        {

            constexpr std::array<std::string_view, static_cast<size_t>(LoggingLevel::_Count)> levels_array = GetLoggingLevelsArray();
            constexpr size_t max_levels_length = std::max_element(levels_array.begin(), levels_array.end(),
                [](std::string_view a, std::string_view b) -> bool
                { 
                    return a.length() < b.length(); 
                })->length();

            constexpr std::array<std::string_view, static_cast<size_t>(LoggingClassification::_Count)> classifications_array = GetLoggingClassificationsArray();
            constexpr size_t max_classifications_length = std::max_element(classifications_array.begin(), classifications_array.end(),
                [](std::string_view a, std::string_view b) -> bool
                { 
                    return a.length() < b.length(); 
                })->length();

            // NOTE(Chris): Reserve 4 more for the brackets.
            constexpr size_t final_length = max_levels_length + max_classifications_length + 4;
            return final_length;

        }

    private:
        std::shared_mutex mutex;
        std::deque<LoggingMessage> messages;

    private:
        LoggingManager() = default;
        ~LoggingManager() = default;

};
