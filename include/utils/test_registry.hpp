#pragma once
#include <utils/defs.hpp>
#include <functional>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>

class TestInterface
{
    public:
        virtual ~TestInterface() = default;
        virtual void run() = 0;

        bool pass               = false;
        real32_t time_start     = 0.0f;
        real32_t time_end       = 0.0f;
        real32_t time_elapsed   = 0.0f;

        inline std::string formatted_result(const std::string &test_name, bool show_memory = false) const
        {
            std::stringstream format;
            format << "[" << (pass ? "PASS" : "FAIL") << "] : " << test_name << "(" << time_elapsed << "ms)";
            return format.str();
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
            const bool result = this->test_function(this->test_parameter); 
            this->pass = result;
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

    private:
        using TestGroupVector_t = std::vector<std::pair<std::string, TestInterface*>>;
        std::vector<TestInterface*> tests;
        std::unordered_map<std::string, TestGroupVector_t> test_groups;

};

#define SIMPLEX_ENABLE_TESTS 1
#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1

#   define SIMPLEX_REGISTER_GENERIC_TEST(name, fn, ptype, ...) \
        static bool _simplex_reg_##name = (TestRegistry::GetInstance()\
            .register_test<ptype>(#name, fn, {__VA_ARGS__}), true)

#   define SIMPLEX_REGISTER_GROUPED_TEST(group, name, fn, ptype, ...) \
        static bool _simplex_reg_##name = (TestRegistry::GetInstance()\
            .register_test<ptype>(#group, #name, fn, {__VA_ARGS__}), true)

#else
#   define SIMPLEX_REGISTER_GENERIC_TEST(name, fn, ptype, ...)
#   define SIMPLEX_REGISTER_GROUPED_TEST(group, name, fn, ptype, ...)
#endif