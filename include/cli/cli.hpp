#pragma once
#include <utils/defs.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

// ---------------------------------------------------------------------------
// CLI Parser
// ---------------------------------------------------------------------------
//
//  Rules are registered at compile time (or during program startup) against
//  three discrete entities:
//
//      (1) Flags          : single-character, case-sensitive, no parameters.
//                           e.g. "-rM" parses as flags 'r' and 'M'.
//      (2) Arguments      : "--name" style, case-INsensitive, may take
//                           zero or more parameters.
//      (3) Positionals    : indexed by their absolute position in argv
//                           (index 0 is the program name).
//
//  Parameters may be classified as strings, paths, integers, floats, or hex
//  values.  Integer parameters may be suffixed with KB, MB, or GB which
//  multiply by 1024, 1024^2, and 1024^3 respectively.  Suffixes must be
//  attached directly to the number (no whitespace).
//
//  Any parse-time failure raises a CLIParseException.
//
// ---------------------------------------------------------------------------

enum class CLIValueType
{
    Any,        // Type deduced on the fly.
    String,
    Path,
    Integer,    // Plain integer, optionally with a KB/MB/GB postfix.
    Float,
    Hex,
};

struct CLIValue
{
    CLIValueType    type            = CLIValueType::String;
    std::string     raw;            // Original token text (unmodified).
    int64_t         integer_value   = 0;
    double          float_value     = 0.0;
};

struct CLIFlagRule
{
    char            letter;
    std::string     description;
};

struct CLIArgumentRule
{
    std::string                 name;           // Stored lower-case (case-insensitive).
    std::vector<CLIValueType>   parameters;     // Expected parameter types; empty == none.
    std::string                 description;
    bool                        required = false;
};

struct CLIPositionalRule
{
    int32_t         index;
    CLIValueType    type;
    std::string     description;
};

class CLIParseException : public std::runtime_error
{
    public:
        inline CLIParseException(const std::string &message)
            : std::runtime_error(message) { }
};

class CLIParser
{
    public:
        CLIParser(int argc, char **argv);
        ~CLIParser() = default;

        // --- Rule registration -------------------------------------------------
        void    add_flag_rule       (char letter,              const std::string &description);
        void    add_argument_rule   (const std::string &name,  const std::vector<CLIValueType> &parameters,
                                                               const std::string &description,
                                                               bool required = false);
        void    add_positional_rule (int32_t index,            CLIValueType type,
                                                               const std::string &description);

        // --- Parsing -----------------------------------------------------------
        void    parse();

        // --- Raw argc/argv access ---------------------------------------------
        inline int          get_argc() const { return this->argc; }
        inline char       **get_argv() const { return this->argv; }
        inline const char  *get_arg(int index) const
                            { return (index >= 0 && index < this->argc) ? this->argv[index] : nullptr; }

        // --- Query -------------------------------------------------------------
        bool                            has_flag(char letter) const;
        bool                            has_argument(const std::string &name) const;
        const std::vector<CLIValue>    *get_argument(const std::string &name) const;
        const CLIValue                 *get_positional(int32_t index) const;

        // --- Output ------------------------------------------------------------
        void    print_header() const;
        void    print_help() const;

    private:
        int                                                     argc;
        char                                                  **argv;

        std::vector<CLIFlagRule>                                flag_rules;
        std::vector<CLIArgumentRule>                            argument_rules;
        std::vector<CLIPositionalRule>                          positional_rules;

        std::vector<char>                                       parsed_flags;
        std::unordered_map<std::string, std::vector<CLIValue>>  parsed_arguments;
        std::unordered_map<int32_t, CLIValue>                   parsed_positionals;

        // --- Helpers -----------------------------------------------------------
        static std::string                  to_lower(const std::string &input);
        static bool                         looks_like_argument(const std::string &token);
        static bool                         looks_like_flag_group(const std::string &token);
        static CLIValue                     classify(const std::string &token, CLIValueType expected);
        static bool                         try_parse_integer(const std::string &token, int64_t &out);
        static bool                         try_parse_hex(const std::string &token, int64_t &out);
        static bool                         try_parse_float(const std::string &token, double &out);
        static bool                         verify_well_formed_path(const std::string &token);
        static const char                  *type_name(CLIValueType type);

        const CLIArgumentRule              *find_argument_rule(const std::string &name_lower) const;
        const CLIPositionalRule            *find_positional_rule(int32_t index) const;
        bool                                is_known_flag(char letter) const;
};
