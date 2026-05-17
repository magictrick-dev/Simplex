#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <iostream>
#include <functional>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <cstdint>

// Per-test snapshot of the memory subsystem, sampled around the test body.
// 'bytes_retained' is the net change in live memory (a leak if positive),
// while 'balanced' reflects whether every byte allocated was also released.
struct TestMemoryStatistics
{
    size_t  bytes_allocated = 0;
    size_t  bytes_released  = 0;
    size_t  bytes_peak      = 0;
    int64_t bytes_retained  = 0;
    bool    balanced        = true;
};

static inline std::string
format_byte_count(int64_t bytes)
{

    int64_t magnitude   = (bytes < 0) ? -bytes : bytes;
    const char *sign    = (bytes < 0) ? "-" : "";

    std::stringstream out;
    if (magnitude < 1024)
        out << sign << magnitude << "B";
    else if (magnitude < 1024 * 1024)
        out << sign << std::fixed << std::setprecision(1) << (magnitude / 1024.0) << "KB";
    else
        out << sign << std::fixed << std::setprecision(1) << (magnitude / (1024.0 * 1024.0)) << "MB";

    return out.str();

}

// Column layout shared by the table header and every test row so that the
// status, timing and memory figures line up vertically before the name.
// Column 0 is the failure marker (" X "); the table is borderless and
// starts flush against the first console column.
static constexpr int test_column_count    = 9;
static constexpr int test_column_widths[] = { 3, 6, 6, 8, 8, 8, 9, 4, 38 };

static inline std::string
test_table_rule()
{

    std::string rule;
    for (int i = 0; i < test_column_count; ++i)
    {

        if (i > 0)
            rule += "-+-";
        rule += std::string(test_column_widths[i], '-');

    }
    return rule;

}

static inline std::string
test_table_row(const std::string cells[test_column_count])
{

    static const bool column_left[test_column_count] =
        { true, true, false, false, false, false, false, false, true };

    std::stringstream row;
    for (int i = 0; i < test_column_count; ++i)
    {

        if (i > 0)
            row << " | ";

        std::string text = cells[i];
        if ((int)text.size() > test_column_widths[i])
            text = text.substr(0, test_column_widths[i]);

        row << (column_left[i] ? std::left : std::right)
            << std::setw(test_column_widths[i]) << text;

    }

    return row.str();

}

class TestInterface
{
    public:
        virtual ~TestInterface() = default;
        virtual void run() = 0;

        bool                    pass            = false;
        real32_t                time_elapsed    = 0.0f;
        TestMemoryStatistics    memory_stats    = {};

        inline std::string formatted_row(const std::string &test_name) const
        {

            const std::string cells[test_column_count] =
            {
                (this->pass ? "" : " X "),
                (this->pass ? "PASS" : "FAIL"),
                std::to_string((long)this->time_elapsed) + "ms",
                format_byte_count(this->memory_stats.bytes_allocated),
                format_byte_count(this->memory_stats.bytes_released),
                format_byte_count(this->memory_stats.bytes_peak),
                format_byte_count(this->memory_stats.bytes_retained),
                (this->memory_stats.balanced ? "OK" : "BAD"),
                test_name
            };

            return test_table_row(cells);

        }

};

template <typename param_t>
class TestHarness : public TestInterface
{
    public:
        using fn_t = std::function<bool(const param_t&)>;

        inline TestHarness(fn_t fn, param_t parameter) : test_function(fn), test_parameter(parameter) { }
        inline virtual ~TestHarness() { }

        inline virtual void run() override
        {

            size_t allocated_before = simplex_memory_get_allocations_total();
            size_t released_before  = simplex_memory_get_releases_total();
            size_t live_before      = simplex_memory_get_live();
            simplex_memory_reset_peak();

            auto start = std::chrono::high_resolution_clock::now();
            const bool result = this->test_function(this->test_parameter);
            auto end = std::chrono::high_resolution_clock::now();

            size_t allocated_after  = simplex_memory_get_allocations_total();
            size_t released_after   = simplex_memory_get_releases_total();
            size_t live_after       = simplex_memory_get_live();
            size_t peak_during      = simplex_memory_get_peak();

            this->time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
            this->pass = result;

            this->memory_stats.bytes_allocated  = allocated_after - allocated_before;
            this->memory_stats.bytes_released   = released_after - released_before;
            this->memory_stats.bytes_peak       = peak_during - live_before;
            this->memory_stats.bytes_retained   = (int64_t)live_after - (int64_t)live_before;
            this->memory_stats.balanced         = (this->memory_stats.bytes_allocated ==
                                                   this->memory_stats.bytes_released);

        }


    private:
        fn_t test_function;
        param_t test_parameter;
};

class TestRegistry
{
    private:
        inline TestRegistry() = default;
        inline TestRegistry(TestRegistry &&other) = default;

        inline ~TestRegistry() = default;

        inline TestRegistry& operator=(const TestRegistry& right) = default;

    public:
        static inline TestRegistry& GetInstance() { static TestRegistry instance; return instance; }
        static inline void Purge() { for (auto t : GetInstance().tests) { delete t; } }

        template <typename TestType, typename ParameterType>
        inline void register_test(const char *name, 
                                  ParameterType parameter, 
                                  std::function<void(const ParameterType&)> test)
        {
            TestType *test_instance = new TestType(test, parameter);
            this->tests.emplace_back(test_instance);
            this->test_groups["General"].emplace_back(name, test_instance);
        }

        template <typename ParameterType>
        inline void register_test(const char *name, 
                                  std::function<bool(const ParameterType&)> test,
                                  ParameterType parameter)
        {
            TestInterface *test_instance = new TestHarness<ParameterType>(test, parameter);
            this->tests.emplace_back(test_instance);
            this->test_groups["General"].emplace_back(name, test_instance);
        }

        template <typename ParameterType>
        inline void register_test(const char* group, const char *name,
                                  std::function<bool(const ParameterType&)> test,
                                  ParameterType parameter)
        {
            TestInterface *test_instance = new TestHarness<ParameterType>(test, parameter);
            this->tests.emplace_back(test_instance);
            this->test_groups[group].emplace_back(name, test_instance);
        }

        inline const auto& get_all() const { return this->test_groups; }
        inline const auto& get_from(const char* group) { return this->test_groups[group]; }

        inline static bool RunEverything()
        {

            // TODO(Chris): We can use a thread pool for each group to speed this up.
            // NOTE(Chris): Some of these tests will take awhile since they are I/O bound.
            //              The big ones is the RDView Tokenizer / Parsing validation.

            auto &instance = GetInstance();

            const auto& test_groups = instance.get_all();
            
            size_t tests_completed  = 0;
            size_t tests_failed     = 0;
            size_t tests_unbalanced = 0;
            size_t total_allocated  = 0;
            size_t peak_live        = simplex_memory_get_live();

            const std::string rule = test_table_rule();

            for (const auto& [group_name, tests] : test_groups)
            {

                const std::string header_cells[test_column_count] =
                    { "", "STATUS", "TIME", "ALLOC", "FREED", "PEAK", "NET", "BAL", "TEST" };

                std::cout << group_name << std::endl;
                std::cout << rule << std::endl;
                std::cout << test_table_row(header_cells) << std::endl;
                std::cout << rule << std::endl;

                for (const auto& [name, test] : tests)
                {

                    test->run();
                    tests_completed++;

                    total_allocated += test->memory_stats.bytes_allocated;
                    if (!test->memory_stats.balanced) tests_unbalanced++;

                    size_t test_peak = simplex_memory_get_peak();
                    if (test_peak > peak_live) peak_live = test_peak;

                    if (test->pass == false) tests_failed++;

                    std::cout << test->formatted_row(name) << std::endl;

                }

                std::cout << rule << std::endl;

            }

            std::cout << "\n";
            std::cout << "Test Results: " << tests_completed - tests_failed << "/" 
                      << tests_completed << " tests" << std::endl;
            std::cout << "Test Memory Results: peak live " << format_byte_count(peak_live)
                      << ", total allocated " << format_byte_count(total_allocated)
                      << ", " << tests_unbalanced << " unbalanced" << std::endl;
            return (tests_failed == 0);

        }

    private:
        using TestGroupVector_t = std::vector<std::pair<std::string, TestInterface*>>;
        std::vector<TestInterface*> tests;
        std::unordered_map<std::string, TestGroupVector_t> test_groups;

};

// TODO(Chris): We will need to move this macro define stuff off to the build system.
#define SIMPLEX_ENABLE_TESTS 1
#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1

#   define SIMPLEX_TEST_NAME_CONCAT_IMPL(a, b) a##b
#   define SIMPLEX_TEST_NAME_CONCAT(a, b) SIMPLEX_TEST_NAME_CONCAT_IMPL(a, b)
#   define SIMPLEX_TEST_NAME(base) SIMPLEX_TEST_NAME_CONCAT(base, __COUNTER__)

#   define SIMPLEX_REGISTER_GENERIC_TEST(name, fn, ptype, ...) \
        static bool SIMPLEX_TEST_NAME(_simplex_reg_) = (TestRegistry::GetInstance()\
            .register_test<ptype>(name, fn, {__VA_ARGS__}), true)

#   define SIMPLEX_REGISTER_GROUPED_TEST(group, name, fn, ptype, ...) \
        static bool SIMPLEX_TEST_NAME(_simplex_reg_) = (TestRegistry::GetInstance()\
            .register_test<ptype>(group, name, fn, {__VA_ARGS__}), true)

#else
#   define SIMPLEX_REGISTER_GENERIC_TEST(name, fn, ptype, ...)
#   define SIMPLEX_REGISTER_GROUPED_TEST(group, name, fn, ptype, ...)
#endif