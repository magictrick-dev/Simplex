#pragma once
#include <utils/defs.hpp>

#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <simplex/array.hpp>
#include <simplex/dynamic_string.hpp>
#include <simplex/static_queue.hpp>
#include <simplex/static_stack.hpp>
#include <simplex/hashed_sparse_map.hpp>

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

        spx::dynamic_string<char> thread;
        spx::dynamic_string<char> message;

    };

    class logger
    {

        using time_point_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

        public:

            static inline void 
            process_message_queue()
            {

            }

            static inline void
            set_thread_name(spx::string_view<char> name)
            {

            }

            static inline constexpr spx::array<spx::string_view<char>, static_cast<size_t>(logging_level::_count)>
            get_logging_levels_array()
            {

                constexpr size_t required_size = static_cast<size_t>(logging_level::_count);
                constexpr spx::array levels
                {
                    spx::string_view<char>("None"),
                    spx::string_view<char>("Debug"),
                    spx::string_view<char>("Diagnostic"),
                    spx::string_view<char>("Information"),
                    spx::string_view<char>("Warning"),
                    spx::string_view<char>("Critical"),
                    spx::string_view<char>("Error"),
                };

                static_assert(levels.size() == required_size, "Mismatched levels to implementation count.");
                return levels;

            }

            static inline constexpr spx::array<spx::string_view<char>, static_cast<size_t>(logging_classification::_count)>
            get_logging_classifications_array()
            {

            }

        private:
            static inline constexpr size_t max_messages = 4096;
            static inline std::shared_mutex mutex;
            static inline spx::static_queue<message, max_messages> messages;
            static inline time_point_t begin = std::chrono::high_resolution_clock::now();
            static inline spx::hashed_sparse_map<std::thread::id, spx::dynamic_string<char>> thread_names;

             logger() = default;
            ~logger() = default;
            logger(const logger& other) = delete;
            logger(logger&& other) = delete;
            logger& operator=(const logger& other) = delete;
            logger& operator=(logger&& other) = delete;

    };

};