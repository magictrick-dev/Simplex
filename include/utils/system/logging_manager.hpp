#pragma once
#include <utils/defs.hpp>

#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <deque>
#include <queue>
#include <array>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <sstream>

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
    _Count,
};

struct LoggingMessage
{

    real32_t issued;

    LoggingLevel level;
    LoggingClassification classification;

    std::string thread;
    std::string message;

};

class LoggingManager
{

    public:
        static inline LoggingManager& Get()
        {
            static LoggingManager manager = {};
            return manager;
        }

        template <LoggingLevel l, LoggingClassification c, typename... Args> static inline void 
        DispatchLog(std::string_view mformat, Args&&... args)
        {
            
            LoggingManager &manager = Get();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<real32_t> time_difference = end - manager.start; 
            real32_t elapsed = time_difference.count();
            constexpr real32_t epsilon_zero = 0.01f;
            if (elapsed < epsilon_zero) elapsed = 0.0f;

            auto thread_id_awkward = std::this_thread::get_id();
            size_t thread_id = std::hash<std::thread::id>{}(thread_id_awkward);
            auto result = Threadnames.find(thread_id);
            
            std::string_view thread = "Other";
            if (result != Threadnames.end()) thread = result->second;

            LoggingMessage message_log  = {};
            message_log.classification  = c;
            message_log.level           = l;
            message_log.message         = std::vformat(mformat, std::make_format_args(std::forward<Args>(args)...));
            message_log.thread          = thread;
            message_log.issued          = elapsed;

            {

                std::unique_lock lock(manager.mutex);
                manager.messages.emplace_back(message_log);
                manager.queued_messages.push(message_log);

                if (manager.messages.size() > MaxMessages) manager.messages.pop_front();

            }
            

        }

        template <typename... Args> static inline void
        DispatchDebug(std::string_view mformat, Args&&... args)
        {
            DispatchLog<LoggingLevel::Debug, LoggingClassification::Engine>(mformat, args...);
        }

        template <typename... Args> static inline void
        DispatchDiagnostic(std::string_view mformat, Args&&... args)
        {
            DispatchLog<LoggingLevel::Diagnostic, LoggingClassification::Engine>(mformat, args...);
        }

        template <typename... Args> static inline void
        DispatchInformation(std::string_view mformat, Args&&... args)
        {
            DispatchLog<LoggingLevel::Information, LoggingClassification::Engine>(mformat, args...);
        }

        template <typename... Args> static inline void
        DispatchWarning(std::string_view mformat, Args&&... args)
        {
            DispatchLog<LoggingLevel::Warning, LoggingClassification::Engine>(mformat, args...);
        }

        template <typename... Args> static inline void
        DispatchCritical(std::string_view mformat, Args&&... args)
        {
            DispatchLog<LoggingLevel::Critical, LoggingClassification::Engine>(mformat, args...);
        }

        template <typename... Args> static inline void
        DispatchError(std::string_view mformat, Args&&... args)
        {
            DispatchLog<LoggingLevel::Error, LoggingClassification::Engine>(mformat, args...);
        }

        static inline void 
        ProcessMessageQueue()
        {


            LoggingManager &manager = Get();
            std::unique_lock lock(manager.mutex);
            while (!manager.queued_messages.empty())
            {

                const auto &current_message = manager.queued_messages.front();
                std::string_view level = GetLoggingLevelString(current_message.level);
                std::string_view classification = GetLoggingClassificationString(current_message.classification);

                std::stringstream header;
                header      << "[" << level << "]" 
                            << "[" << classification << "]"
                            << "[" << current_message.thread << "]"
                            << "(" << std::setprecision(4) << current_message.issued << "s)"
                            << " : ";

                std::cout << header.view();
                size_t length = header.view().length();

                std::stringstream body { current_message.message };

                bool processed = false;
                std::string current_line;
                while (std::getline(body, current_line))
                {
                    if (processed == true) std::cout << std::setw(length) << "";
                    else processed = true;
                    std::cout << current_line << "\n";
                }

                manager.queued_messages.pop();

            }

        }

        static inline constexpr std::array<std::string_view, static_cast<size_t>(LoggingLevel::_Count)>
        GetLoggingLevelsArray()
        {

            constexpr std::array levels =
            {
                std::string_view("None"),
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
                std::string_view(""),
                std::string_view("Internal"),
                std::string_view("Engine"),
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

        static inline void
        ClassifyThreadname(std::string_view name)
        {

            auto &manager = Get();
            std::unique_lock lock(manager.mutex);

            auto thread_id_awkward = std::this_thread::get_id();
            size_t thread_id = std::hash<std::thread::id>{}(thread_id_awkward);
            Threadnames[thread_id] = name; // Allows for overwrites.

        }

        static constexpr size_t MaxMessages = 1024;
        static inline std::unordered_map<size_t, std::string_view> Threadnames;

    private:
        std::shared_mutex mutex;
        std::deque<LoggingMessage> messages;
        std::queue<LoggingMessage> queued_messages;
        std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

    private:
        LoggingManager() = default;
        ~LoggingManager() = default;

};
