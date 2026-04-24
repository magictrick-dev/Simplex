#include <cli/cli.hpp>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

CLIParser::
CLIParser(int argc, char **argv)
    : argc(argc), argv(argv)
{

}

// ---------------------------------------------------------------------------
// Rule registration
// ---------------------------------------------------------------------------

void CLIParser::
add_flag_rule(char letter, const std::string &description)
{
    this->flag_rules.push_back({ letter, description });
}

void CLIParser::
add_argument_rule(const std::string &name, const std::vector<CLIValueType> &parameters, const std::string &description)
{
    CLIArgumentRule rule;
    rule.name        = CLIParser::to_lower(name);
    rule.parameters  = parameters;
    rule.description = description;
    this->argument_rules.push_back(rule);
}

void CLIParser::
add_positional_rule(int32_t index, CLIValueType type, const std::string &description)
{
    this->positional_rules.push_back({ index, type, description });
}

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

void CLIParser::
parse()
{

    this->parsed_flags.clear();
    this->parsed_arguments.clear();
    this->parsed_positionals.clear();

    int i = 1; // Skip program name.
    while (i < this->argc)
    {

        std::string token = this->argv[i];

        if (CLIParser::looks_like_argument(token))
        {

            // --argument-style, case-insensitive.
            std::string name            = CLIParser::to_lower(token);
            const CLIArgumentRule *rule = this->find_argument_rule(name);

            size_t expected = (rule != nullptr) ? rule->parameters.size() : 0;

            std::vector<CLIValue> collected;
            int j = i + 1;
            while (j < this->argc)
            {

                std::string next = this->argv[j];
                if (CLIParser::looks_like_argument(next))           break;
                if (CLIParser::looks_like_flag_group(next))         break;

                // If a rule exists, cap at the rule's expected count; unexpected
                // extras are left for subsequent parse loops (likely a positional).
                if (rule != nullptr && collected.size() >= expected) break;

                CLIValueType expected_type = CLIValueType::Any;
                if (rule != nullptr && collected.size() < expected)
                    expected_type = rule->parameters[collected.size()];

                collected.push_back(CLIParser::classify(next, expected_type));
                j++;

            }

            // Under-satisfied rule?
            if (rule != nullptr && collected.size() < expected)
            {
                std::ostringstream os;
                os << "Argument '" << token << "' expected "
                   << expected << " parameter(s), received " << collected.size() << ".";
                throw CLIParseException(os.str());
            }

            this->parsed_arguments[name] = std::move(collected);
            i = j;
            continue;

        }

        if (CLIParser::looks_like_flag_group(token))
        {

            // -rM => 'r', 'M' (case-sensitive).
            for (size_t k = 1; k < token.size(); ++k)
            {

                char c = token[k];

                // Validate against known flag rules if any are declared. If no
                // flag rules were registered we permissively accept everything.
                if (!this->flag_rules.empty() && !this->is_known_flag(c))
                {
                    std::ostringstream os;
                    os << "Unknown flag '-" << c << "' in token '" << token << "'.";
                    throw CLIParseException(os.str());
                }

                this->parsed_flags.push_back(c);

            }

            i++;
            continue;

        }

        // Otherwise: positional parameter at absolute argv index i.
        const CLIPositionalRule *rule   = this->find_positional_rule(i);
        CLIValueType             expect = (rule != nullptr) ? rule->type : CLIValueType::Any;
        this->parsed_positionals[i]     = CLIParser::classify(token, expect);
        i++;

    }

}

// ---------------------------------------------------------------------------
// Query
// ---------------------------------------------------------------------------

bool CLIParser::
has_flag(char letter) const
{
    for (char c : this->parsed_flags) if (c == letter) return true;
    return false;
}

bool CLIParser::
has_argument(const std::string &name) const
{
    return this->parsed_arguments.find(CLIParser::to_lower(name)) != this->parsed_arguments.end();
}

const std::vector<CLIValue> *CLIParser::
get_argument(const std::string &name) const
{
    auto it = this->parsed_arguments.find(CLIParser::to_lower(name));
    if (it == this->parsed_arguments.end()) return nullptr;
    return &it->second;
}

const CLIValue *CLIParser::
get_positional(int32_t index) const
{
    auto it = this->parsed_positionals.find(index);
    if (it == this->parsed_positionals.end()) return nullptr;
    return &it->second;
}

// ---------------------------------------------------------------------------
// Output
// ---------------------------------------------------------------------------

void CLIParser::
print_header() const
{
    const char *program = (this->argc > 0 && this->argv[0] != nullptr) ? this->argv[0] : "Simplex";
    printf("%s\n", program);
}

void CLIParser::
print_help() const
{

    printf("Usage: <program> [positionals] [flags] [arguments]\n\n");

    if (!this->positional_rules.empty())
    {
        printf("Positionals:\n");
        for (const auto &r : this->positional_rules)
            printf("    [%d] <%s>    %s\n", r.index, CLIParser::type_name(r.type), r.description.c_str());
        printf("\n");
    }

    if (!this->flag_rules.empty())
    {
        printf("Flags:\n");
        for (const auto &r : this->flag_rules)
            printf("    -%c    %s\n", r.letter, r.description.c_str());
        printf("\n");
    }

    if (!this->argument_rules.empty())
    {
        printf("Arguments:\n");
        for (const auto &r : this->argument_rules)
        {
            printf("    %s", r.name.c_str());
            for (CLIValueType t : r.parameters) printf(" <%s>", CLIParser::type_name(t));
            printf("    %s\n", r.description.c_str());
        }
        printf("\n");
    }

}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string CLIParser::
to_lower(const std::string &input)
{
    std::string out = input;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return out;
}

bool CLIParser::
looks_like_argument(const std::string &token)
{
    return token.size() >= 3 && token[0] == '-' && token[1] == '-';
}

bool CLIParser::
looks_like_flag_group(const std::string &token)
{

    // Must begin with a single dash, have at least one character, and not
    // be a negative numeric literal (e.g. "-12", "-3.5", "-0x1F").
    if (token.size() < 2)   return false;
    if (token[0] != '-')    return false;
    if (token[1] == '-')    return false;

    // Reject if the remainder looks numeric (so "-12" is treated as a value).
    int64_t itmp = 0; double ftmp = 0.0;
    if (CLIParser::try_parse_integer(token, itmp)) return false;
    if (CLIParser::try_parse_float  (token, ftmp)) return false;
    if (CLIParser::try_parse_hex    (token, itmp)) return false;

    // Each remaining character must be a letter.
    for (size_t i = 1; i < token.size(); ++i)
        if (!std::isalpha((unsigned char)token[i])) return false;

    return true;

}

bool CLIParser::
try_parse_hex(const std::string &token, int64_t &out)
{

    size_t i = 0;
    bool negative = false;
    if (i < token.size() && (token[i] == '+' || token[i] == '-'))
    {
        negative = (token[i] == '-');
        i++;
    }

    if (i + 1 >= token.size())              return false;
    if (token[i] != '0')                    return false;
    if (token[i+1] != 'x' && token[i+1] != 'X') return false;
    i += 2;

    if (i >= token.size())                  return false;

    int64_t value = 0;
    for (; i < token.size(); ++i)
    {
        char c = token[i];
        int digit;
        if      (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F') digit = 10 + (c - 'A');
        else return false;
        value = (value << 4) | digit;
    }

    out = negative ? -value : value;
    return true;

}

bool CLIParser::
try_parse_integer(const std::string &token, int64_t &out)
{

    if (token.empty()) return false;

    size_t i = 0;
    bool negative = false;
    if (token[i] == '+' || token[i] == '-')
    {
        negative = (token[i] == '-');
        i++;
    }
    if (i >= token.size()) return false;

    // Must start with a digit.
    if (!std::isdigit((unsigned char)token[i])) return false;

    int64_t value = 0;
    while (i < token.size() && std::isdigit((unsigned char)token[i]))
    {
        value = value * 10 + (token[i] - '0');
        i++;
    }

    // Optional postfix: KB / MB / GB (no intervening whitespace).
    int64_t multiplier = 1;
    if (i < token.size())
    {

        std::string suffix = token.substr(i);
        std::string upper;
        upper.reserve(suffix.size());
        for (char c : suffix) upper.push_back((char)std::toupper((unsigned char)c));

        if      (upper == "KB") multiplier = 1024LL;
        else if (upper == "MB") multiplier = 1024LL * 1024LL;
        else if (upper == "GB") multiplier = 1024LL * 1024LL * 1024LL;
        else return false;

    }

    out = (negative ? -value : value) * multiplier;
    return true;

}

bool CLIParser::
try_parse_float(const std::string &token, double &out)
{

    if (token.empty())                      return false;
    // Require a decimal point or exponent to disambiguate from an integer.
    bool has_marker = false;
    for (char c : token) if (c == '.' || c == 'e' || c == 'E') { has_marker = true; break; }
    if (!has_marker)                        return false;

    const char *begin = token.c_str();
    char *end = nullptr;
    double v = std::strtod(begin, &end);
    if (end != begin + token.size())        return false;

    out = v;
    return true;

}

bool CLIParser::
verify_well_formed_path(const std::string &token)
{

    if (token.empty()) return false;
    for (unsigned char c : token)
    {
        if (c < 0x20)                               return false; // control chars.
        if (c == '<' || c == '>' || c == '|')       return false;
        if (c == '?' || c == '*')                   return false;
        if (c == '"')                               return false;
    }

    return true;

}

CLIValue CLIParser::
classify(const std::string &token, CLIValueType expected)
{

    CLIValue v;
    v.raw = token;

    auto set_int   = [&](int64_t x){ v.type = CLIValueType::Integer; v.integer_value = x; v.float_value = (double)x; };
    auto set_hex   = [&](int64_t x){ v.type = CLIValueType::Hex;     v.integer_value = x; v.float_value = (double)x; };
    auto set_float = [&](double x) { v.type = CLIValueType::Float;   v.float_value   = x; v.integer_value = (int64_t)x; };

    int64_t itmp = 0;
    double  ftmp = 0.0;

    switch (expected)
    {

        case CLIValueType::Integer:
        {
            if (!CLIParser::try_parse_integer(token, itmp))
                throw CLIParseException("Expected integer parameter, got '" + token + "'.");
            set_int(itmp);
            return v;
        }

        case CLIValueType::Hex:
        {
            if (!CLIParser::try_parse_hex(token, itmp))
                throw CLIParseException("Expected hex parameter, got '" + token + "'.");
            set_hex(itmp);
            return v;
        }

        case CLIValueType::Float:
        {
            if (CLIParser::try_parse_float(token, ftmp))        { set_float(ftmp); return v; }
            if (CLIParser::try_parse_integer(token, itmp))      { set_float((double)itmp); return v; }
            throw CLIParseException("Expected float parameter, got '" + token + "'.");
        }

        case CLIValueType::Path:
        {
            if (!CLIParser::verify_well_formed_path(token))
                throw CLIParseException("Malformed path parameter '" + token + "'.");
            v.type = CLIValueType::Path;
            return v;
        }

        case CLIValueType::String:
        {
            v.type = CLIValueType::String;
            return v;
        }

        case CLIValueType::Any:
        default:
        {
            if (CLIParser::try_parse_hex    (token, itmp))      { set_hex(itmp);    return v; }
            if (CLIParser::try_parse_float  (token, ftmp))      { set_float(ftmp);  return v; }
            if (CLIParser::try_parse_integer(token, itmp))      { set_int(itmp);    return v; }
            // Heuristic: treat tokens containing path separators as paths.
            if (token.find('/') != std::string::npos || token.find('\\') != std::string::npos)
            {
                if (!CLIParser::verify_well_formed_path(token))
                    throw CLIParseException("Malformed path token '" + token + "'.");
                v.type = CLIValueType::Path;
                return v;
            }
            v.type = CLIValueType::String;
            return v;
        }

    }

}

const CLIArgumentRule *CLIParser::
find_argument_rule(const std::string &name_lower) const
{
    for (const auto &r : this->argument_rules) if (r.name == name_lower) return &r;
    return nullptr;
}

const CLIPositionalRule *CLIParser::
find_positional_rule(int32_t index) const
{
    for (const auto &r : this->positional_rules) if (r.index == index) return &r;
    return nullptr;
}

bool CLIParser::
is_known_flag(char letter) const
{
    for (const auto &r : this->flag_rules) if (r.letter == letter) return true;
    return false;
}

const char *CLIParser::
type_name(CLIValueType type)
{
    switch (type)
    {
        case CLIValueType::String:  return "string";
        case CLIValueType::Path:    return "path";
        case CLIValueType::Integer: return "integer";
        case CLIValueType::Float:   return "float";
        case CLIValueType::Hex:     return "hex";
        case CLIValueType::Any:     return "any";
    }
    return "any";
}
