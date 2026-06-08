#pragma once
#include <utils/defs.hpp>

#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <simplex/array.hpp>
#include <simplex/static_queue.hpp>
#include <simplex/static_stack.hpp>
#include <simplex/static_deque.hpp>
#include <simplex/hashed_sparse_map.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace spx
{

    enum class logging_level : size_t
    {
        naked,
        debug,
        diagnostic,
        information,
        warning,
        critical,
        error,
        _count,
    };

    enum class logging_classification : size_t
    {
        undefined,
        internal,
        engine,
        _count,
    };

    struct message
    {

        real32_t issued;
        logging_level level;
        logging_classification classification;

        std::string thread;
        std::string message;

    };

    class logger
    {

        public:
            static inline constexpr size_t max_messages = 4096;
            using time_point_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

        public:

            template <logging_level l, logging_classification c, typename... Args> static inline void
            dispatch_message(std::string_view format, Args&&... args)
            {

                // Get the time.
                constexpr real32_t epsilon_zero = 0.01f;
                time_point_t end = std::chrono::high_resolution_clock::now();
                real32_t elapsed_time = std::chrono::duration<real32_t>(end - begin).count();
                if (elapsed_time < epsilon_zero) elapsed_time = 0.0f;

                // Acquire the name of the thread.
                std::string_view thread_name = get_thread_name();

                // Format the message.
                message log = {};
                log.issued          = elapsed_time;
                log.level           = l;
                log.classification  = c;
                log.message         = std::vformat(format, std::make_format_args(std::forward<Args>(args)...));
                log.thread          = thread_name;

                // Push the message.
                push_message_log(log);

            }

            template <typename... Args> static inline void
            dispatch_debug_log(std::string_view format, Args&&... args)
            {
                dispatch_message<logging_level::debug, logging_classification::engine>(format, args...);
            }

            template <typename... Args> static inline void
            dispatch_diagnostic_log(std::string_view format, Args&&... args)
            {
                dispatch_message<logging_level::diagnostic, logging_classification::engine>(format, args...);
            }

            template <typename... Args> static inline void
            dispatch_information_log(std::string_view format, Args&&... args)
            {
                dispatch_message<logging_level::information, logging_classification::engine>(format, args...);
            }

            template <typename... Args> static inline void
            dispatch_warning_log(std::string_view format, Args&&... args)
            {
                dispatch_message<logging_level::warning, logging_classification::engine>(format, args...);
            }

            template <typename... Args> static inline void
            dispatch_critical_log(std::string_view format, Args&&... args)
            {
                dispatch_message<logging_level::critical, logging_classification::engine>(format, args...);
            }

            template <typename... Args> static inline void
            dispatch_error_log(std::string_view format, Args&&... args)
            {
                dispatch_message<logging_level::error, logging_classification::engine>(format, args...);
            }

            static inline void 
            process_message_queue()
            {

                std::unique_lock lock(mutex);
                while(!messages.empty())
                {

                    const auto &current_message = messages.front();
                    std::string_view level = get_logging_level_string(current_message.level);
                    std::string_view classification = get_logging_classification_string(current_message.classification);

                    std::stringstream header;
                    header      << "[" << level << "]" 
                                << "[" << classification << "]"
                                << "[" << current_message.thread.c_str() << "]"
                                << "(" << std::setprecision(4) << current_message.issued << "s)"
                                << " : ";

                    std::cout << header.view();
                    size_t length = header.view().length();

                    std::stringstream body { current_message.message.c_str() };

                    bool processed = false;
                    std::string current_line;
                    while (std::getline(body, current_line))
                    {
                        if (processed == true) std::cout << std::setw(length) << "";
                        else processed = true;
                        std::cout << current_line << "\n";
                    }

                    messages.pop();

                }

            }

            static inline void
            set_thread_name(std::string_view name)
            {

                std::unique_lock lock(mutex);
                const std::thread::id thread_id = std::this_thread::get_id();
                thread_names[thread_id] = name;
                
            }

            static inline std::string_view
            get_thread_name()
            {

                std::shared_lock lock(mutex);
                const std::thread::id thread_id = std::this_thread::get_id();
                if (thread_names.contains(thread_id))
                {
                    return thread_names[thread_id];
                }

                return "Unnamed";

            }

            static inline constexpr spx::array<std::string_view, static_cast<size_t>(logging_level::_count)>
            get_logging_levels_array()
            {

                constexpr size_t required_size = static_cast<size_t>(logging_level::_count);
                constexpr spx::array levels
                {
                    std::string_view("None"),
                    std::string_view("Debug"),
                    std::string_view("Diagnostic"),
                    std::string_view("Information"),
                    std::string_view("Warning"),
                    std::string_view("Critical"),
                    std::string_view("Error"),
                };

                static_assert(levels.size() == required_size, "Mismatched levels to implementation count.");
                return levels;

            }

            static inline constexpr spx::array<std::string_view, static_cast<size_t>(logging_classification::_count)>
            get_logging_classifications_array()
            {

                constexpr size_t required_size = static_cast<size_t>(logging_classification::_count);
                constexpr spx::array classifications
                {
                    std::string_view(""),
                    std::string_view("Internal"),
                    std::string_view("Engine"),
                };

                static_assert(classifications.size() == required_size, "Mismatched classifications to implementation count.");
                return classifications;

            }

            static inline constexpr std::string_view
            get_logging_level_string(logging_level level)
            {
                return get_logging_levels_array()[static_cast<size_t>(level)];
            }

            static inline constexpr std::string_view
            get_logging_classification_string(logging_classification classification)
            {
                return get_logging_classifications_array()[static_cast<size_t>(classification)];
            }


            /// @brief Provides a way for retrieving the rolling message queue for displaying in a GUI.
            /// @return A static double-ended queue of the messages.
            static inline spx::static_deque<message, max_messages>
            get_messages()
            {
                std::shared_lock lock(mutex);
                return rolling_messages;
            }

        private:
            static inline void
            push_message_log(message& log)
            {
                std::unique_lock lock(mutex);
                if (messages.size() >= max_messages) messages.pop(); // Oops, too many in queue. 
                if (rolling_messages.size() >= max_messages) rolling_messages.pop_back();
                messages.emplace(log);
                rolling_messages.emplace_front(log);
            }

        private:
            static inline std::shared_mutex mutex;
            static inline spx::static_queue<message, max_messages> messages;
            static inline spx::static_deque<message, max_messages> rolling_messages;
            static inline time_point_t begin = std::chrono::high_resolution_clock::now();
            static inline spx::hashed_sparse_map<std::thread::id, std::string> thread_names;

             logger() = default;
            ~logger() = default;
            logger(const logger& other) = delete;
            logger(logger&& other) = delete;
            logger& operator=(const logger& other) = delete;
            logger& operator=(logger&& other) = delete;

    };

};