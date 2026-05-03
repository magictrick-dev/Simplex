#include <parsers/rdview.hpp>

// -------------------------------------------------------------------------------------------------
// RDViewTokenizer
// -------------------------------------------------------------------------------------------------

enum RDViewDFAState
{
    RDViewDFAState_WhitespaceEntry,
    RDViewDFAState_WhitespaceSingle,
    RDViewDFAState_WhitespaceCommentEntry,
    RDViewDFAState_WhitespaceCommentBody,
    RDViewDFAState_WhitespaceExit,

    RDViewDFAState_NumberEntry,
    RDViewDFAState_NumberIntegerBody,
    RDViewDFAState_NumberRealBody,
    RDViewDFAState_NumberHexBody,
    RDViewDFAState_NumberExit,

    RDViewDFAState_StringEntry,
    RDViewDFAState_StringBody,
    RDViewDFAState_StringExit,

    RDViewDFAState_IdentifierEntry,
    RDViewDFAState_IdentifierBody,
    RDViewDFAState_IdentifierExit,
};

RDViewTokenizer::
RDViewTokenizer(std::string_view source_contents, std::filesystem::path source_path)
{

    this->source = source_contents;
    this->path = source_path;

    this->offset = 0;
    this->step = 0;
    this->line = 1;
    this->column = 1;

    this->previous_token = &this->tokens[0];
    this->current_token = &this->tokens[1];
    this->next_token = &this->tokens[2];

    this->previous_token->type = RDViewTokenType_EOF;
    this->current_token->type = RDViewTokenType_EOF;
    this->next_token->type = RDViewTokenType_EOF;

    this->shift();
    this->shift();

}

RDViewTokenizer::
~RDViewTokenizer()
{

}

RDViewToken RDViewTokenizer:: 
get_previous_token() const
{
    return *this->previous_token;
}

RDViewToken RDViewTokenizer:: 
get_current_token() const
{
    return *this->current_token;
}

RDViewToken RDViewTokenizer:: 
get_next_token() const
{
    return *this->next_token;
}

bool RDViewTokenizer:: 
shift()
{

    *this->previous_token = *this->current_token;
    *this->current_token = *this->next_token;
    this->match(this->next_token);

    bool is_eof = this->current_token_is(RDViewTokenType_EOF);
    return is_eof;

}

bool RDViewTokenizer:: 
previous_token_is(RDViewTokenType type) const
{
    bool result = this->previous_token->type == type;
    return result;
}

bool RDViewTokenizer:: 
current_token_is(RDViewTokenType type) const
{
    bool result = this->current_token->type == type;
    return result;
}

bool RDViewTokenizer:: 
next_token_is(RDViewTokenType type) const
{
    bool result = this->next_token->type == type;
    return result;
}

bool RDViewTokenizer::
is_current_keyword(RDViewKeywordType type) const
{
    if (this->current_token_is(RDViewTokenType_Keyword))
        return (this->current_token->keyword.type == type);
    return false;
}

void RDViewTokenizer::
format_token_as(RDViewToken *token, RDViewTokenType type)
{
    token->type                     = type;
    token->offset                   = this->offset;
    token->line                     = this->line;
    token->column                   = this->column;
    token->length                   = this->step - this->offset;
    token->source_file_contents     = this->source;
    token->source_file_path         = this->path;
}

void RDViewTokenizer::
match(RDViewToken *token)
{

    // Chomps the whitespace and fast-exits if we reached EOF.
    this->handle_whitespace();
    if (this->is_eof()) 
    {
        this->format_token_as(token, RDViewTokenType_EOF);
        return;
    }

    if (this->handle_numbers(token)) return;
    else if (this->handle_strings(token)) return;
    else if (this->handle_identifiers(token)) return;

    this->consume();
    this->format_token_as(token, RDViewTokenType_Invalid);
    this->synchronize();

}

void RDViewTokenizer::
handle_whitespace()
{

    RDViewDFAState state = RDViewDFAState_WhitespaceEntry;

    while (!this->is_eof())
    {

        char c = this->peak();
        switch (state)
        {

            // NOTE(Chris): The DFA may re-enter the entry node after processing single whitespaces
            //              or comments as a means of a catch-all.
            case RDViewDFAState_WhitespaceEntry:
            {

                // We are at EOF.
                if (c == '\0')
                {
                    state = RDViewDFAState_WhitespaceExit;
                }
                
                // Standard whitespace.
                else if (isspace(c))
                {
                    state = RDViewDFAState_WhitespaceSingle;
                }
                
                // Comments.
                else if (c == '#')
                {
                    state = RDViewDFAState_WhitespaceCommentEntry;
                }

                // Non-whitespace character.
                else
                {
                    state = RDViewDFAState_WhitespaceExit;
                }
            } break;

            case RDViewDFAState_WhitespaceSingle:
            {
                this->consume();
                state = RDViewDFAState_WhitespaceEntry;
            } break;

            case RDViewDFAState_WhitespaceCommentEntry:
            {
                this->consume();
                state = RDViewDFAState_WhitespaceCommentBody;
            } break;

            case RDViewDFAState_WhitespaceCommentBody:
            {
                if (c == '\n')
                {
                    state = RDViewDFAState_WhitespaceEntry;
                }
                else
                {
                    this->consume();
                }
            } break;

            case RDViewDFAState_WhitespaceExit:
            {
                this->synchronize();
                return;
            } break;

            default:
            {
                // NOTE(Chris): We really messed up if we hit this, so hard assert here.
                SIMPLEX_NO_REACH("Reached an unexpected RDView DFA state.");
            } break;

        }

    }

    this->synchronize();

}

void RDViewTokenizer::
parse_number_value(RDViewToken *token, RDViewTokenType token_type, std::string_view raw)
{

    // Strip digit-separator single-quotes before handing to the standard library.
    std::string digits;
    digits.reserve(raw.size());
    for (char c : raw)
    {
        if (c != '\'') digits += c;
    }

    if (token_type == RDViewTokenType_Integer)
    {
        // Base 0: auto-detects 0x/0X prefix for hex, plain digits for decimal.
        token->integer.value = std::stoll(digits, nullptr, 0);
    }
    else if (token_type == RDViewTokenType_Real)
    {
        token->real.value = std::stod(digits);
    }
    else
    {
        SIMPLEX_NO_REACH("Invalid token type encountered in numbers DFA.");
    }

}

bool RDViewTokenizer::
handle_numbers(RDViewToken *token)
{

    RDViewDFAState state = RDViewDFAState_NumberEntry;
    RDViewTokenType token_type = RDViewTokenType_Integer;

    while (!this->is_eof())
    {

        char c = this->peak();
        switch (state)
        {

            // NOTE(Chris): Dispatches on the first character(s) to determine the number kind.
            //              Uses one or two characters of lookahead to reject non-number starts
            //              without consuming anything, since handle_numbers must be side-effect
            //              free on failure.
            case RDViewDFAState_NumberEntry:
            {

                // Sign prefix: only valid when followed by a digit, or by '.' and then a digit.
                if (c == '+' || c == '-')
                {
                    char next = this->peak_at(1);
                    if (isdigit((unsigned char)next))
                    {
                        this->consume();
                        state = RDViewDFAState_NumberIntegerBody;
                    }
                    else if (next == '.' && isdigit((unsigned char)this->peak_at(2)))
                    {
                        this->consume(); // sign
                        this->consume(); // dot
                        token_type = RDViewTokenType_Real;
                        state = RDViewDFAState_NumberRealBody;
                    }
                    else
                    {
                        return false;
                    }
                }

                // Leading dot: only valid when immediately followed by a digit.
                else if (c == '.')
                {
                    if (isdigit((unsigned char)this->peak_at(1)))
                    {
                        this->consume();
                        token_type = RDViewTokenType_Real;
                        state = RDViewDFAState_NumberRealBody;
                    }
                    else
                    {
                        return false;
                    }
                }

                // Digit: check for hex prefix (0x/0X + at least one hex digit), else decimal.
                else if (isdigit((unsigned char)c))
                {
                    if (c == '0'
                        && (this->peak_at(1) == 'x' || this->peak_at(1) == 'X')
                        && isxdigit((unsigned char)this->peak_at(2)))
                    {
                        this->consume(); // '0'
                        this->consume(); // 'x' or 'X'
                        state = RDViewDFAState_NumberHexBody;
                    }
                    else
                    {
                        // NOTE(Chris): The digit is not consumed here; IntegerBody handles it.
                        state = RDViewDFAState_NumberIntegerBody;
                    }
                }

                // Not a number start.
                else
                {
                    return false;
                }

            } break;

            // Decimal integer digits. A single-quote is consumed only when the next
            // character is also a digit (digit separator, never leading or trailing).
            // A dot transitions to real only when followed by a digit.
            case RDViewDFAState_NumberIntegerBody:
            {
                if (isdigit((unsigned char)c))
                {
                    this->consume();
                }
                else if (c == '\'' && isdigit((unsigned char)this->peak_at(1)))
                {
                    this->consume();
                }
                else if (c == '.' && isdigit((unsigned char)this->peak_at(1)))
                {
                    this->consume();
                    token_type = RDViewTokenType_Real;
                    state = RDViewDFAState_NumberRealBody;
                }
                else
                {
                    state = RDViewDFAState_NumberExit;
                }
            } break;

            // Real fractional digits. Same separator rules as integers; no second dot.
            case RDViewDFAState_NumberRealBody:
            {
                if (isdigit((unsigned char)c))
                {
                    this->consume();
                }
                else if (c == '\'' && isdigit((unsigned char)this->peak_at(1)))
                {
                    this->consume();
                }
                else
                {
                    state = RDViewDFAState_NumberExit;
                }
            } break;

            // Hex digits (after 0x/0X already consumed). Same single-quote separator rules.
            case RDViewDFAState_NumberHexBody:
            {
                if (isxdigit((unsigned char)c))
                {
                    this->consume();
                }
                else if (c == '\'' && isxdigit((unsigned char)this->peak_at(1)))
                {
                    this->consume();
                }
                else
                {
                    state = RDViewDFAState_NumberExit;
                }
            } break;

            case RDViewDFAState_NumberExit:
            {
                this->format_token_as(token, token_type);
                this->parse_number_value(token, token_type, this->source.substr(this->offset, token->length));
                this->synchronize();
                return true;
            } break;

            default:
            {
                SIMPLEX_NO_REACH("Reached an unexpected RDView DFA state.");
            } break;

        }

    }

    // Reached EOF while consuming a number. Entry means we consumed nothing.
    if (state == RDViewDFAState_NumberEntry) return false;

    this->format_token_as(token, token_type);
    this->parse_number_value(token, token_type, this->source.substr(this->offset, token->length));
    this->synchronize();
    return true;

}

bool RDViewTokenizer::
handle_strings(RDViewToken *token)
{

    RDViewDFAState state = RDViewDFAState_StringEntry;

    while (!this->is_eof())
    {

        char c = this->peak();
        switch (state)
        {

            // Only a double-quote opens a string; anything else is not our token to handle.
            case RDViewDFAState_StringEntry:
            {
                if (c == '"')
                {
                    this->consume();
                    state = RDViewDFAState_StringBody;
                }
                else
                {
                    return false;
                }
            } break;

            case RDViewDFAState_StringBody:
            {
                // Closing quote, valid string.
                if (c == '"')
                {
                    this->consume();
                    state = RDViewDFAState_StringExit;
                }

                // Newline before close, invalid. Do not consume the newline so that
                // line tracking and whitespace handling stay correct.
                else if (c == '\n')
                {
                    this->format_token_as(token, RDViewTokenType_Invalid);
                    this->synchronize();
                    return true;
                }

                // Regular character, consume and stay in body.
                else
                {
                    this->consume();
                }
            } break;

            case RDViewDFAState_StringExit:
            {
                // offset points to the opening '"', step is one past the closing '"'.
                // length = step - offset (set by format_token_as), so the interior is
                // offset+1 with length-2 characters.
                this->format_token_as(token, RDViewTokenType_String);
                token->string.value = this->source.substr(this->offset + 1, token->length - 2);
                this->synchronize();
                return true;
            } break;

            default:
            {
                SIMPLEX_NO_REACH("Reached an unexpected RDView DFA state.");
            } break;

        }

    }

    // Reached EOF without a closing quote.
    // If still in Entry, nothing was consumed — not our token.
    if (state == RDViewDFAState_StringEntry)
        return false;

    this->format_token_as(token, RDViewTokenType_Invalid);
    this->synchronize();
    return true;

}

bool RDViewTokenizer::
handle_identifiers(RDViewToken *token)
{

    RDViewDFAState state = RDViewDFAState_IdentifierEntry;

    while (!this->is_eof())
    {

        char c = this->peak();
        switch (state)
        {

            // NOTE(Chris): '$' is valid only as the leading character, per the RDView
            //              parameter specification. Letters and underscores also start
            //              an identifier; the first character is not consumed here,
            //              IdentifierBody handles it so the DFA stays uniform.
            case RDViewDFAState_IdentifierEntry:
            {
                if (c == '$')
                {
                    this->consume();
                    state = RDViewDFAState_IdentifierBody;
                }
                else if (isalpha((unsigned char)c) || c == '_')
                {
                    state = RDViewDFAState_IdentifierBody;
                }
                else
                {
                    return false;
                }
            } break;

            // Consume standard C-style identifier continuation characters.
            // '$' is intentionally excluded here, it is only legal at the start.
            case RDViewDFAState_IdentifierBody:
            {
                if (isalnum((unsigned char)c) || c == '_')
                {
                    this->consume();
                }
                else
                {
                    state = RDViewDFAState_IdentifierExit;
                }
            } break;

            case RDViewDFAState_IdentifierExit:
            {
                this->format_token_as(token, RDViewTokenType_Identifier);
                RDViewKeywordType keyword_type = this->classify_identifier(token);
                if (keyword_type != RDViewKeywordType_Invalid)
                {
                    token->keyword.type = keyword_type;
                    token->type = RDViewTokenType_Keyword;
                }
                else
                {
                    token->identifier.value = this->source.substr(this->offset, token->length);
                }
                this->synchronize();
                return true;
            } break;

            default:
            {
                SIMPLEX_NO_REACH("Reached an unexpected RDView DFA state.");
            } break;

        }

    }

    // Reached EOF while consuming an identifier. Entry means nothing was consumed.
    if (state == RDViewDFAState_IdentifierEntry)
        return false;

    this->format_token_as(token, RDViewTokenType_Identifier);
    RDViewKeywordType keyword_type = this->classify_identifier(token);
    if (keyword_type != RDViewKeywordType_Invalid)
    {
        token->keyword.type = keyword_type;
        token->type = RDViewTokenType_Keyword;
    }
    else
    {
        token->identifier.value = this->source.substr(this->offset, token->length);
    }
    this->synchronize();
    return true;

}

RDViewKeywordType RDViewTokenizer::
classify_identifier(RDViewToken *token)
{

    // Static table: built once, shared across all tokenizer instances.
    // Keyed on string_view so lookup requires no allocation.
    static const std::unordered_map<std::string_view, RDViewKeywordType> keywords =
    {
        // General Commands
        { "Display",        RDViewKeywordType_Display        },
        { "Format",         RDViewKeywordType_Format         },
        { "Include",        RDViewKeywordType_Include        },

        // File Structuring
        { "FrameBegin",     RDViewKeywordType_FrameBegin     },
        { "FrameEnd",       RDViewKeywordType_FrameEnd       },
        { "WorldBegin",     RDViewKeywordType_WorldBegin     },
        { "WorldEnd",       RDViewKeywordType_WorldEnd       },
        { "ObjectBegin",    RDViewKeywordType_ObjectBegin    },
        { "ObjectEnd",      RDViewKeywordType_ObjectEnd      },
        { "ObjectInstance", RDViewKeywordType_ObjectInstance },

        // Drawing Attributes
        { "Background",     RDViewKeywordType_Background     },
        { "Color",          RDViewKeywordType_Color          },
        { "Opacity",        RDViewKeywordType_Opacity        },

        // Options
        { "OptionArray",    RDViewKeywordType_OptionArray    },
        { "OptionBool",     RDViewKeywordType_OptionBool     },
        { "OptionList",     RDViewKeywordType_OptionList     },
        { "OptionReal",     RDViewKeywordType_OptionReal     },
        { "OptionString",   RDViewKeywordType_OptionString   },

        // Camera
        { "CameraAt",       RDViewKeywordType_CameraAt       },
        { "CameraEye",      RDViewKeywordType_CameraEye      },
        { "CameraFOV",      RDViewKeywordType_CameraFOV      },
        { "CameraUp",       RDViewKeywordType_CameraUp       },
        { "Clipping",       RDViewKeywordType_Clipping       },

        // Geometry
        { "Point",          RDViewKeywordType_Point          },
        { "Line",           RDViewKeywordType_Line           },
        { "Circle",         RDViewKeywordType_Circle         },
        { "Fill",           RDViewKeywordType_Fill           },
        { "Disk",           RDViewKeywordType_Disk           },
        { "Cone",           RDViewKeywordType_Cone           },
        { "Cube",           RDViewKeywordType_Cube           },
        { "Cylinder",       RDViewKeywordType_Cylinder       },
        { "Sphere",         RDViewKeywordType_Sphere         },
        { "Torus",          RDViewKeywordType_Torus          },
        { "Tube",           RDViewKeywordType_Tube           },
        { "Paraboloid",     RDViewKeywordType_Paraboloid     },
        { "Hyperboloid",    RDViewKeywordType_Hyperboloid    },
        { "SqSphere",       RDViewKeywordType_SqSphere       },
        { "SqTorus",        RDViewKeywordType_SqTorus        },
        { "PointSet",       RDViewKeywordType_PointSet       },
        { "LineSet",        RDViewKeywordType_LineSet        },
        { "PolySet",        RDViewKeywordType_PolySet        },
        { "Curve",          RDViewKeywordType_Curve          },
        { "Patch",          RDViewKeywordType_Patch          },
        { "Subdivision",    RDViewKeywordType_Subdivision    },

        // Geometric Transformations
        { "Translate",      RDViewKeywordType_Translate      },
        { "Scale",          RDViewKeywordType_Scale          },
        { "Rotate",         RDViewKeywordType_Rotate         },
        { "Matrix",         RDViewKeywordType_Matrix         },
        { "XformPush",      RDViewKeywordType_XformPush      },
        { "XformPop",       RDViewKeywordType_XformPop       },

        // Lighting
        { "AmbientLight",   RDViewKeywordType_AmbientLight   },
        { "FarLight",       RDViewKeywordType_FarLight       },
        { "PointLight",     RDViewKeywordType_PointLight     },
        { "ConeLight",      RDViewKeywordType_ConeLight      },

        // Surface Attributes
        { "Ka",             RDViewKeywordType_Ka             },
        { "Kd",             RDViewKeywordType_Kd             },
        { "Ks",             RDViewKeywordType_Ks             },
        { "Specular",       RDViewKeywordType_Specular       },
        { "Surface",        RDViewKeywordType_Surface        },

        // Attribute Mapping
        { "MapLoad",        RDViewKeywordType_MapLoad        },
        { "Map",            RDViewKeywordType_Map            },
        { "MapSample",      RDViewKeywordType_MapSample      },
        { "MapBound",       RDViewKeywordType_MapBound       },
        { "MapBorder",      RDViewKeywordType_MapBorder      },
    };

    // offset..offset+length spans the identifier just scanned. No allocation needed.
    std::string_view text = this->source.substr(this->offset, token->length);
    auto it = keywords.find(text);
    if (it != keywords.end())
        return it->second;

    return RDViewKeywordType_Invalid;

}

void RDViewTokenizer::
synchronize()
{
    this->offset = step;
}

char RDViewTokenizer::
peak() const
{
    if (this->step >= this->source.length())
        return '\0';
    return this->source[this->step];
}

char RDViewTokenizer::
peak_at(size_t count) const
{
    if (this->step + count >= this->source.length())
        return '\0';
    return this->source[this->step + count];
}

void RDViewTokenizer::
consume()
{
    if (this->step < this->source.length())
    {
        if (this->source[this->step] == '\n')
        {
            this->line++;
            this->column = 1;
        }
        this->step++;
    }
}

bool RDViewTokenizer::
is_eof() const
{
    bool result = (this->source[this->step] == '\0' || this->step >= this->source.length());
    return result;
}

// -------------------------------------------------------------------------------------------------
// RDViewParser
// -------------------------------------------------------------------------------------------------

RDViewParser::
RDViewParser()
{

}

RDViewParser::
~RDViewParser()
{

    for (auto &node : this->nodes)
    {
        destroy_node(node);
        node = nullptr;
    }

    this->nodes.clear();

}

void RDViewParser::
synchronize_to(RDViewTokenType token_type)
{

    while (true)
    {

        auto current_token = this->tokenizer->get_current_token();
        if (current_token.type == RDViewTokenType_EOF || 
            current_token.type == token_type) break;
        this->consume();

    }

}

bool RDViewParser:: 
is_previous_token(RDViewTokenType token_type) const
{
    const bool result = (this->tokenizer->previous_token_is(token_type));
    return result;
}

bool RDViewParser:: 
is_current_token(RDViewTokenType token_type) const
{
    const bool result = (this->tokenizer->current_token_is(token_type));
    return result;
}

bool RDViewParser:: 
is_next_token(RDViewTokenType token_type) const
{
    const bool result = (this->tokenizer->next_token_is(token_type));
    return result;
}

bool RDViewParser::
expect_keyword(RDViewKeywordType keyword_type, std::string error)
{
    const bool result = (this->tokenizer->is_current_keyword(keyword_type));
    if (result == false) this->throw_error<RDViewParserErrorUC>(this->tokenizer->get_current_token(), error);
    return result;
}

bool RDViewParser::
expect_type(RDViewTokenType token_type)
{
    const bool result = (this->tokenizer->current_token_is(token_type));
    if (result == false) this->throw_error<RDViewParserErrorUT>(this->tokenizer->get_current_token());
    return result;
}

RDViewToken RDViewParser::
fetch_type_and_consume(RDViewTokenType token_type)
{
    auto result = this->tokenizer->get_current_token();
    if (!this->tokenizer->current_token_is(token_type)) this->throw_error<RDViewParserErrorUT>(result);
    this->consume();
    return result;
}

void RDViewParser::
consume()
{
    this->tokenizer->shift();
}

real32_t RDViewParser::
fetch_numerical_and_consume()
{
    auto token = this->tokenizer->get_current_token();
    if (this->is_current_token(RDViewTokenType_Real))
    {
        this->consume();
        return (real32_t)token.real.value;
    }
    else if (this->is_current_token(RDViewTokenType_Integer))
    {
        this->consume();
        return (real32_t)token.integer.value;
    }
    this->throw_error<RDViewParserErrorUT>(token);
    return 0.0f;
}

bool RDViewParser::
open_include(const std::filesystem::path& path)
{

    std::string key = path.string();

    for (const auto& active : this->include_chain)
    {
        if (active == key) return false;
    }

    // Record the dependency edge from the currently parsing file to this one.
    if (!this->include_chain.empty())
        this->include_graph[this->include_chain.back()].push_back(key);

    // Ensure every opened file has its own entry, even if it includes nothing.
    this->include_graph.try_emplace(key);

    this->include_chain.push_back(key);
    return true;

}

void RDViewParser::
close_include(const std::filesystem::path& path)
{
    SIMPLEX_ASSERT(!this->include_chain.empty());
    SIMPLEX_ASSERT(this->include_chain.back() == path.string());
    this->include_chain.pop_back();
}

bool RDViewParser::
includes_file(const std::filesystem::path& path) const
{
    return this->include_graph.count(path.string()) > 0;
}

const std::vector<std::string>* RDViewParser::
get_direct_includes(const std::filesystem::path& path) const
{
    auto it = this->include_graph.find(path.string());
    if (it == this->include_graph.end()) return nullptr;
    return &it->second;
}

bool RDViewParser::
has_transitive_dependency(const std::filesystem::path& from, const std::filesystem::path& to) const
{

    std::string from_key = from.string();
    std::string to_key   = to.string();

    auto start = this->include_graph.find(from_key);
    if (start == this->include_graph.end()) return false;

    std::unordered_set<std::string> visited;
    std::vector<std::string> stack(start->second.begin(), start->second.end());

    while (!stack.empty())
    {
        std::string current = std::move(stack.back());
        stack.pop_back();

        if (current == to_key) return true;
        if (!visited.insert(current).second) continue;

        auto it = this->include_graph.find(current);
        if (it != this->include_graph.end())
        {
            for (const auto& dep : it->second)
                stack.push_back(dep);
        }
    }

    return false;

}

const std::vector<std::string>& RDViewParser::
get_include_chain() const
{
    return this->include_chain;
}

static inline void
print_include_node(
    const std::unordered_map<std::string, std::vector<std::string>>& graph,
    std::ostream& stream,
    const std::string& file,
    const std::string& prefix,
    bool is_last,
    std::unordered_set<std::string>& visited)
{

    stream << prefix << (is_last ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 " : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ");
    stream << std::filesystem::path(file).filename().string();

    if (!visited.insert(file).second)
    {
        stream << "  (already shown)" << std::endl;
        return;
    }

    stream << std::endl;

    auto it = graph.find(file);
    if (it == graph.end() || it->second.empty())
        return;

    const auto& deps = it->second;
    std::string child_prefix = prefix + (is_last ? "    " : "\xe2\x94\x82   ");

    for (size_t i = 0; i < deps.size(); ++i)
        print_include_node(graph, stream, deps[i], child_prefix, i == deps.size() - 1, visited);

}

void RDViewParser::
print_include_graph(std::ostream& stream) const
{

    if (this->include_graph.empty())
    {
        stream << "(include graph is empty)" << std::endl;
        return;
    }

    std::unordered_set<std::string> non_roots;
    for (const auto& [file, deps] : this->include_graph)
    {
        for (const auto& dep : deps)
            non_roots.insert(dep);
    }

    std::vector<std::string> roots;
    for (const auto& [file, deps] : this->include_graph)
    {
        if (non_roots.find(file) == non_roots.end())
            roots.push_back(file);
    }

    std::sort(roots.begin(), roots.end());

    for (size_t i = 0; i < roots.size(); ++i)
    {
        const auto& root = roots[i];
        stream << std::filesystem::path(root).filename().string() << std::endl;

        std::unordered_set<std::string> visited;
        visited.insert(root);

        auto it = this->include_graph.find(root);
        if (it != this->include_graph.end())
        {
            const auto& deps = it->second;
            for (size_t j = 0; j < deps.size(); ++j)
                print_include_node(this->include_graph, stream, deps[j], "", j == deps.size() - 1, visited);
        }

        if (i + 1 < roots.size())
            stream << std::endl;
    }

}

RDViewNodeInterface* RDViewParser::
match_root()
{
    
    try
    {

        // NOTE(Chris): We are enforcing that display and format commands exist in the script here.
        //              This is a deviation from the original specification.
        auto display_node = this->match_display();
        auto format_node = this->match_format();
        auto body_node = this->match_body();

        RDViewNodeRoot *root_node = this->create_node<RDViewNodeRoot>();
        root_node->display = display_node;
        root_node->format = format_node;
        root_node->body = body_node;
        return root_node;

    }
    catch (RDViewParserError &e)
    {
        std::cout << e.what() << std::endl;
        return NULL;
    }

}

RDViewNodeInterface* RDViewParser::
match_body()
{

    RDViewNodeBody *body = this->create_node<RDViewNodeBody>();

    while (!this->is_current_token(RDViewTokenType_EOF))
    {

        auto current_token = this->tokenizer->get_current_token();

        try
        {

            this->expect_type(RDViewTokenType_Keyword);
            RDViewKeywordType current_keyword = current_token.keyword.type;

            RDViewNodeInterface *result = NULL;
            switch (current_keyword)
            {
                case RDViewKeywordType_Include:         { result = this->match_include();       } break;
                case RDViewKeywordType_ObjectBegin:     { result = this->match_object();        } break;
                case RDViewKeywordType_OptionArray:     { result = this->match_option_array();  } break;
                case RDViewKeywordType_OptionBool:      { result = this->match_option_bool();   } break;
                case RDViewKeywordType_OptionList:      { result = this->match_option_list();   } break;
                case RDViewKeywordType_OptionReal:      { result = this->match_option_real();   } break;
                case RDViewKeywordType_OptionString:    { result = this->match_option_string(); } break;
                case RDViewKeywordType_FrameBegin:      { result = this->match_frame();         } break;
                default:
                {
                    // NOTE(Chris): Any other commands are invalid in this context.
                    this->consume();
                    this->throw_error<RDViewParserErrorUC>(current_token, "script body");
                }
            }

            // NOTE(Chris): No matter what, we should get a valid token back, the try/catch
            //              is responsible for resynchronizing correctly.
            SIMPLEX_CHECK_PTR(result);
            body->children.push_back(result);

        }
        catch (RDViewParserError &e)
        {
            std::cout << e.what() << std::endl;
            this->synchronize_to(RDViewTokenType_Keyword);
        }

    }

    return body;

}

RDViewNodeInterface* RDViewParser::
match_include()
{
    
    auto command = this->tokenizer->get_current_token();
    this->expect_keyword(RDViewKeywordType_Include, "script root (expected 'Include')");
    this->consume();

    auto path = this->fetch_type_and_consume(RDViewTokenType_String);

    std::string user_path(path.string.value);
    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(user_path);

    if (!std::filesystem::exists(canonical_path))
    {
        this->throw_error_and_recover<RDViewParserErrorINF>(command, user_path);
    }

    // TODO(Chris): Push a new parser, match on it, then set.

    RDViewNodeInclude *include = this->create_node<RDViewNodeInclude>();
    include->input_path = user_path;
    include->canonical_path = canonical_path;

    return include;
}

RDViewNodeInterface* RDViewParser::
match_display()
{

    auto command = this->tokenizer->get_current_token();
    this->expect_keyword(RDViewKeywordType_Display, "script header (expected 'Display')");
    this->consume();

    auto name   = this->fetch_type_and_consume(RDViewTokenType_String);
    auto format = this->fetch_type_and_consume(RDViewTokenType_String);
    auto mode   = this->fetch_type_and_consume(RDViewTokenType_String);

    RDViewDisplayType format_type = RDViewNodeDisplay::map_display_type(format.string.value);
    RDViewModeType mode_type = RDViewNodeDisplay::map_mode_type(mode.string.value);
    
    if (format_type == RDViewDisplayType_Invalid)
        this->throw_error<RDViewParserErrorICF>(format, "invalid display format type.");

    if (mode_type == RDViewDisplayType_Invalid)
        this->throw_error<RDViewParserErrorICF>(mode, "invalid display mode type.");

    RDViewNodeDisplay *display = this->create_node<RDViewNodeDisplay>();
    display->name = name.string.value;
    display->format = format_type;
    display->mode = mode_type;

    return display;

}

RDViewNodeInterface* RDViewParser::
match_format()
{

    this->expect_keyword(RDViewKeywordType_Format, "script header (expected 'Format')");
    this->consume();

    auto width = this->fetch_type_and_consume(RDViewTokenType_Integer);
    auto height = this->fetch_type_and_consume(RDViewTokenType_Integer);

    RDViewNodeFormat *format = this->create_node<RDViewNodeFormat>();
    format->width = width.integer.value;
    format->height = height.integer.value;

    return format;

}

RDViewNodeInterface* RDViewParser::
match_object()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_frame()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_world()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_camera()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_geometry()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_transforms()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_lighting()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_option_array()
{
    this->expect_keyword(RDViewKeywordType_OptionArray, "option (expected 'OptionArray')");
    this->consume();

    auto name  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto count = this->fetch_type_and_consume(RDViewTokenType_Integer);

    RDViewNodeOptionArray *node = this->create_node<RDViewNodeOptionArray>();
    node->name = std::string(name.string.value);
    node->values.reserve((size_t)count.integer.value);
    for (int64_t i = 0; i < count.integer.value; ++i)
        node->values.push_back(this->fetch_numerical_and_consume());
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_bool()
{
    this->expect_keyword(RDViewKeywordType_OptionBool, "option (expected 'OptionBool')");
    this->consume();

    auto name = this->fetch_type_and_consume(RDViewTokenType_String);

    auto token = this->tokenizer->get_current_token();
    int32_t bool_value = 0;

    if (this->is_current_token(RDViewTokenType_Integer))
    {
        bool_value = (int32_t)token.integer.value;
        this->consume();
    }
    else if (this->is_current_token(RDViewTokenType_Identifier))
    {
        std::string_view id = token.identifier.value;
        if      (id == "true"  || id == "on")  bool_value = 1;
        else if (id == "false" || id == "off") bool_value = 0;
        else this->throw_error<RDViewParserErrorICF>(token, "expected boolean value (true/false/on/off/integer)");
        this->consume();
    }
    else
    {
        this->throw_error<RDViewParserErrorICF>(token, "expected boolean value");
    }

    RDViewNodeOptionBool *node = this->create_node<RDViewNodeOptionBool>();
    node->name  = std::string(name.string.value);
    node->value = bool_value;
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_list()
{
    this->expect_keyword(RDViewKeywordType_OptionList, "option (expected 'OptionList')");
    this->consume();

    auto name  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto count = this->fetch_type_and_consume(RDViewTokenType_Integer);

    RDViewNodeOptionList *node = this->create_node<RDViewNodeOptionList>();
    node->name = std::string(name.string.value);
    node->values.reserve((size_t)count.integer.value);
    for (int64_t i = 0; i < count.integer.value; ++i)
    {
        auto str = this->fetch_type_and_consume(RDViewTokenType_String);
        node->values.push_back(std::string(str.string.value));
    }
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_real()
{
    this->expect_keyword(RDViewKeywordType_OptionReal, "option (expected 'OptionReal')");
    this->consume();

    auto name = this->fetch_type_and_consume(RDViewTokenType_String);

    RDViewNodeOptionReal *node = this->create_node<RDViewNodeOptionReal>();
    node->name  = std::string(name.string.value);
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_option_string()
{
    this->expect_keyword(RDViewKeywordType_OptionString, "option (expected 'OptionString')");
    this->consume();

    auto name  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto value = this->fetch_type_and_consume(RDViewTokenType_String);

    RDViewNodeOptionString *node = this->create_node<RDViewNodeOptionString>();
    node->name  = std::string(name.string.value);
    node->value = std::string(value.string.value);
    return node;
}

RDViewNodeInterface* RDViewParser::
match_background()
{
    this->expect_keyword(RDViewKeywordType_Background, "frame (expected 'Background')");
    this->consume();

    RDViewNodeBackground *node = this->create_node<RDViewNodeBackground>();
    node->red   = this->fetch_numerical_and_consume();
    node->green = this->fetch_numerical_and_consume();
    node->blue  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_color()
{
    this->expect_keyword(RDViewKeywordType_Color, "expected 'Color'");
    this->consume();

    RDViewNodeColor *node = this->create_node<RDViewNodeColor>();
    node->red   = this->fetch_numerical_and_consume();
    node->green = this->fetch_numerical_and_consume();
    node->blue  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_opacity()
{
    this->expect_keyword(RDViewKeywordType_Opacity, "expected 'Opacity'");
    this->consume();

    RDViewNodeOpacity *node = this->create_node<RDViewNodeOpacity>();
    node->opacity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_at()
{
    this->expect_keyword(RDViewKeywordType_CameraAt, "camera (expected 'CameraAt')");
    this->consume();

    RDViewNodeCameraAt *node = this->create_node<RDViewNodeCameraAt>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_eye()
{
    this->expect_keyword(RDViewKeywordType_CameraEye, "camera (expected 'CameraEye')");
    this->consume();

    RDViewNodeCameraEye *node = this->create_node<RDViewNodeCameraEye>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_fov()
{
    this->expect_keyword(RDViewKeywordType_CameraFOV, "camera (expected 'CameraFOV')");
    this->consume();

    RDViewNodeCameraFOV *node = this->create_node<RDViewNodeCameraFOV>();
    node->FOV = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_camera_up()
{
    this->expect_keyword(RDViewKeywordType_CameraUp, "camera (expected 'CameraUp')");
    this->consume();

    RDViewNodeCameraUp *node = this->create_node<RDViewNodeCameraUp>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_clipping()
{
    this->expect_keyword(RDViewKeywordType_Clipping, "camera (expected 'Clipping')");
    this->consume();

    RDViewNodeClipping *node = this->create_node<RDViewNodeClipping>();
    node->near = this->fetch_numerical_and_consume();
    node->far  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_point()
{
    this->expect_keyword(RDViewKeywordType_Point, "geometry (expected 'Point')");
    this->consume();

    RDViewNodePoint *node = this->create_node<RDViewNodePoint>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_point_set()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_line()
{
    this->expect_keyword(RDViewKeywordType_Line, "geometry (expected 'Line')");
    this->consume();

    RDViewNodeLine *node = this->create_node<RDViewNodeLine>();
    node->x1 = this->fetch_numerical_and_consume();
    node->y1 = this->fetch_numerical_and_consume();
    node->z1 = this->fetch_numerical_and_consume();
    node->x2 = this->fetch_numerical_and_consume();
    node->y2 = this->fetch_numerical_and_consume();
    node->z2 = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_line_set()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_circle()
{
    this->expect_keyword(RDViewKeywordType_Circle, "geometry (expected 'Circle')");
    this->consume();

    RDViewNodeCircle *node = this->create_node<RDViewNodeCircle>();
    node->x      = this->fetch_numerical_and_consume();
    node->y      = this->fetch_numerical_and_consume();
    node->z      = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_fill()
{
    this->expect_keyword(RDViewKeywordType_Fill, "geometry (expected 'Fill')");
    this->consume();

    RDViewNodeFill *node = this->create_node<RDViewNodeFill>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_cone()
{
    this->expect_keyword(RDViewKeywordType_Cone, "geometry (expected 'Cone')");
    this->consume();

    RDViewNodeCone *node = this->create_node<RDViewNodeCone>();
    node->height = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_cube()
{
    this->expect_keyword(RDViewKeywordType_Cube, "geometry (expected 'Cube')");
    this->consume();
    return this->create_node<RDViewNodeCube>();
}

RDViewNodeInterface* RDViewParser::
match_curve()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_cylinder()
{
    this->expect_keyword(RDViewKeywordType_Cylinder, "geometry (expected 'Cylinder')");
    this->consume();

    RDViewNodeCylinder *node = this->create_node<RDViewNodeCylinder>();
    node->radius = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_disk()
{
    this->expect_keyword(RDViewKeywordType_Disk, "geometry (expected 'Disk')");
    this->consume();

    RDViewNodeDisk *node = this->create_node<RDViewNodeDisk>();
    node->height = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_hyperboloid()
{
    this->expect_keyword(RDViewKeywordType_Hyperboloid, "geometry (expected 'Hyperboloid')");
    this->consume();

    RDViewNodeHyperboloid *node = this->create_node<RDViewNodeHyperboloid>();
    node->x1    = this->fetch_numerical_and_consume();
    node->y1    = this->fetch_numerical_and_consume();
    node->z1    = this->fetch_numerical_and_consume();
    node->x2    = this->fetch_numerical_and_consume();
    node->y2    = this->fetch_numerical_and_consume();
    node->z2    = this->fetch_numerical_and_consume();
    node->theta = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_paraboloid()
{
    this->expect_keyword(RDViewKeywordType_Paraboloid, "geometry (expected 'Paraboloid')");
    this->consume();

    RDViewNodeParaboloid *node = this->create_node<RDViewNodeParaboloid>();
    node->radius = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_patch()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_poly_set()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_sphere()
{
    this->expect_keyword(RDViewKeywordType_Sphere, "geometry (expected 'Sphere')");
    this->consume();

    RDViewNodeSphere *node = this->create_node<RDViewNodeSphere>();
    node->radius = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_sq_sphere()
{
    this->expect_keyword(RDViewKeywordType_SqSphere, "geometry (expected 'SqSphere')");
    this->consume();

    auto n_token = this->tokenizer->get_current_token();

    RDViewNodeSqSphere *node = this->create_node<RDViewNodeSqSphere>();
    node->radius = this->fetch_numerical_and_consume();
    n_token      = this->fetch_type_and_consume(RDViewTokenType_Integer);
    node->n      = (real32_t)n_token.integer.value;
    node->e      = this->fetch_numerical_and_consume();
    node->z_min  = this->fetch_numerical_and_consume();
    node->z_max  = this->fetch_numerical_and_consume();
    node->theta  = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_sq_torus()
{
    this->expect_keyword(RDViewKeywordType_SqTorus, "geometry (expected 'SqTorus')");
    this->consume();

    RDViewNodeSqTorus *node = this->create_node<RDViewNodeSqTorus>();
    node->radius_a  = this->fetch_numerical_and_consume();
    node->radius_b  = this->fetch_numerical_and_consume();
    auto n_token    = this->fetch_type_and_consume(RDViewTokenType_Integer);
    node->n         = (real32_t)n_token.integer.value;
    node->e         = this->fetch_numerical_and_consume();
    node->phi_min   = this->fetch_numerical_and_consume();
    node->phi_max   = this->fetch_numerical_and_consume();
    node->theta_max = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_torus()
{
    this->expect_keyword(RDViewKeywordType_Torus, "geometry (expected 'Torus')");
    this->consume();

    RDViewNodeTorus *node = this->create_node<RDViewNodeTorus>();
    node->radius_a  = this->fetch_numerical_and_consume();
    node->radius_b  = this->fetch_numerical_and_consume();
    node->phi_min   = this->fetch_numerical_and_consume();
    node->phi_max   = this->fetch_numerical_and_consume();
    node->theta_max = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_tube()
{
    this->expect_keyword(RDViewKeywordType_Tube, "geometry (expected 'Tube')");
    this->consume();

    RDViewNodeTube *node = this->create_node<RDViewNodeTube>();
    node->x1     = this->fetch_numerical_and_consume();
    node->y1     = this->fetch_numerical_and_consume();
    node->z1     = this->fetch_numerical_and_consume();
    node->x2     = this->fetch_numerical_and_consume();
    node->y2     = this->fetch_numerical_and_consume();
    node->z2     = this->fetch_numerical_and_consume();
    node->radius = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_subdivision()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_object_instance()
{
    SIMPLEX_NO_IMPLEMENTATION("");
    return nullptr;
}

RDViewNodeInterface* RDViewParser::
match_matrix()
{
    this->expect_keyword(RDViewKeywordType_Matrix, "transform (expected 'Matrix')");
    this->consume();

    RDViewNodeMatrix *node = this->create_node<RDViewNodeMatrix>();
    node->values.reserve(16);
    for (int i = 0; i < 16; ++i)
        node->values.push_back(this->fetch_numerical_and_consume());
    return node;
}

RDViewNodeInterface* RDViewParser::
match_rotate()
{
    this->expect_keyword(RDViewKeywordType_Rotate, "transform (expected 'Rotate')");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewRotationAxis> axis_map =
    {
        { "X", RDViewRotationAxis_X },
        { "Y", RDViewRotationAxis_Y },
        { "Z", RDViewRotationAxis_Z },
    };

    auto axis_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto it = axis_map.find(axis_token.string.value);
    if (it == axis_map.end())
        this->throw_error<RDViewParserErrorICF>(axis_token, "expected rotation axis (\"X\", \"Y\", or \"Z\")");

    RDViewNodeRotate *node = this->create_node<RDViewNodeRotate>();
    node->axis  = it->second;
    node->angle = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_scale()
{
    this->expect_keyword(RDViewKeywordType_Scale, "transform (expected 'Scale')");
    this->consume();

    RDViewNodeScale *node = this->create_node<RDViewNodeScale>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_translate()
{
    this->expect_keyword(RDViewKeywordType_Translate, "transform (expected 'Translate')");
    this->consume();

    RDViewNodeTranslate *node = this->create_node<RDViewNodeTranslate>();
    node->x = this->fetch_numerical_and_consume();
    node->y = this->fetch_numerical_and_consume();
    node->z = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_xformpush()
{
    this->expect_keyword(RDViewKeywordType_XformPush, "transform (expected 'XformPush')");
    this->consume();
    return this->create_node<RDViewNodeXformPush>();
}

RDViewNodeInterface* RDViewParser::
match_xformpop()
{
    this->expect_keyword(RDViewKeywordType_XformPop, "transform (expected 'XformPop')");
    this->consume();
    return this->create_node<RDViewNodeXformPop>();
}

RDViewNodeInterface* RDViewParser::
match_ambient_light()
{
    this->expect_keyword(RDViewKeywordType_AmbientLight, "lighting (expected 'AmbientLight')");
    this->consume();

    RDViewNodeAmbientLight *node = this->create_node<RDViewNodeAmbientLight>();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_far_light()
{
    this->expect_keyword(RDViewKeywordType_FarLight, "lighting (expected 'FarLight')");
    this->consume();

    RDViewNodeFarLight *node = this->create_node<RDViewNodeFarLight>();
    node->l_x       = this->fetch_numerical_and_consume();
    node->l_y       = this->fetch_numerical_and_consume();
    node->l_z       = this->fetch_numerical_and_consume();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_point_light()
{
    this->expect_keyword(RDViewKeywordType_PointLight, "lighting (expected 'PointLight')");
    this->consume();

    RDViewNodePointLight *node = this->create_node<RDViewNodePointLight>();
    node->p_x       = this->fetch_numerical_and_consume();
    node->p_y       = this->fetch_numerical_and_consume();
    node->p_z       = this->fetch_numerical_and_consume();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_cone_light()
{
    this->expect_keyword(RDViewKeywordType_ConeLight, "lighting (expected 'ConeLight')");
    this->consume();

    RDViewNodeConeLight *node = this->create_node<RDViewNodeConeLight>();
    node->p_x       = this->fetch_numerical_and_consume();
    node->p_y       = this->fetch_numerical_and_consume();
    node->p_z       = this->fetch_numerical_and_consume();
    node->a_x       = this->fetch_numerical_and_consume();
    node->a_y       = this->fetch_numerical_and_consume();
    node->a_z       = this->fetch_numerical_and_consume();
    node->theta_min = this->fetch_numerical_and_consume();
    node->theta_max = this->fetch_numerical_and_consume();
    node->r         = this->fetch_numerical_and_consume();
    node->g         = this->fetch_numerical_and_consume();
    node->b         = this->fetch_numerical_and_consume();
    node->intensity = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_ka()
{
    this->expect_keyword(RDViewKeywordType_Ka, "surface (expected 'Ka')");
    this->consume();

    RDViewNodeKa *node = this->create_node<RDViewNodeKa>();
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_kd()
{
    this->expect_keyword(RDViewKeywordType_Kd, "surface (expected 'Kd')");
    this->consume();

    RDViewNodeKd *node = this->create_node<RDViewNodeKd>();
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_ks()
{
    this->expect_keyword(RDViewKeywordType_Ks, "surface (expected 'Ks')");
    this->consume();

    RDViewNodeKs *node = this->create_node<RDViewNodeKs>();
    node->value = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_specular()
{
    this->expect_keyword(RDViewKeywordType_Specular, "surface (expected 'Specular')");
    this->consume();

    RDViewNodeSpecular *node = this->create_node<RDViewNodeSpecular>();
    node->r = this->fetch_numerical_and_consume();
    node->g = this->fetch_numerical_and_consume();
    node->b = this->fetch_numerical_and_consume();
    node->n = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_surface()
{
    this->expect_keyword(RDViewKeywordType_Surface, "surface (expected 'Surface')");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewShaderType> shader_map =
    {
        { "Matte",          RDViewShaderType_Matte          },
        { "Metal",          RDViewShaderType_Metal          },
        { "Plastic",        RDViewShaderType_Plastic        },
        { "PaintedPlastic", RDViewShaderType_PaintedPlastic },
    };

    auto shader_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto it = shader_map.find(shader_token.string.value);
    if (it == shader_map.end())
        this->throw_error<RDViewParserErrorICF>(shader_token, "invalid shader type");

    RDViewNodeSurface *node = this->create_node<RDViewNodeSurface>();
    node->shader_type = it->second;
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_load()
{
    this->expect_keyword(RDViewKeywordType_MapLoad, "expected 'MapLoad'");
    this->consume();

    auto path_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto label_token = this->fetch_type_and_consume(RDViewTokenType_String);

    std::string user_path(path_token.string.value);
    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(user_path);

    RDViewNodeMapLoad *node = this->create_node<RDViewNodeMapLoad>();
    node->input_path    = user_path;
    node->canonical_path = canonical_path;
    node->label         = std::string(label_token.string.value);
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map()
{
    this->expect_keyword(RDViewKeywordType_Map, "expected 'Map'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    auto type_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto label_token = this->fetch_type_and_consume(RDViewTokenType_String);

    auto it = map_type_map.find(type_token.string.value);
    if (it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    RDViewNodeMap *node = this->create_node<RDViewNodeMap>();
    node->map_type = it->second;
    node->label    = std::string(label_token.string.value);
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_sample()
{
    this->expect_keyword(RDViewKeywordType_MapSample, "expected 'MapSample'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    static const std::unordered_map<std::string_view, RDViewMapLevelType> level_type_map =
    {
        { "Nearest", RDViewMapLevelType_Nearest },
        { "Linear",  RDViewMapLevelType_Linear  },
    };

    auto type_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto intra_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto inter_token = this->fetch_type_and_consume(RDViewTokenType_String);

    auto type_it = map_type_map.find(type_token.string.value);
    if (type_it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    auto intra_it = level_type_map.find(intra_token.string.value);
    if (intra_it == level_type_map.end())
        this->throw_error<RDViewParserErrorICF>(intra_token, "invalid sample level type");

    auto inter_it = level_type_map.find(inter_token.string.value);
    if (inter_it == level_type_map.end())
        this->throw_error<RDViewParserErrorICF>(inter_token, "invalid sample level type");

    RDViewNodeMapSample *node = this->create_node<RDViewNodeMapSample>();
    node->map_type         = type_it->second;
    node->intra_level_type = intra_it->second;
    node->inter_level_type = inter_it->second;
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_bound()
{
    this->expect_keyword(RDViewKeywordType_MapBound, "expected 'MapBound'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    auto type_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto it = map_type_map.find(type_token.string.value);
    if (it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    RDViewNodeMapBound *node = this->create_node<RDViewNodeMapBound>();
    node->map_type = it->second;
    node->s_min    = this->fetch_numerical_and_consume();
    node->t_min    = this->fetch_numerical_and_consume();
    node->s_max    = this->fetch_numerical_and_consume();
    node->t_max    = this->fetch_numerical_and_consume();
    return node;
}

RDViewNodeInterface* RDViewParser::
match_map_border()
{
    this->expect_keyword(RDViewKeywordType_MapBorder, "expected 'MapBorder'");
    this->consume();

    static const std::unordered_map<std::string_view, RDViewMapType> map_type_map =
    {
        { "none",       RDViewMapType_None       },
        { "TextureMap", RDViewMapType_TextureMap },
        { "BumpMap",    RDViewMapType_BumpMap    },
    };

    static const std::unordered_map<std::string_view, RDViewBorderType> border_type_map =
    {
        { "none",   RDViewMapBorderType_None   },
        { "Clamp",  RDViewMapBorderType_Clamp  },
        { "Repeat", RDViewMapBorderType_Repeat },
    };

    auto type_token  = this->fetch_type_and_consume(RDViewTokenType_String);
    auto horiz_token = this->fetch_type_and_consume(RDViewTokenType_String);
    auto vert_token  = this->fetch_type_and_consume(RDViewTokenType_String);

    auto type_it = map_type_map.find(type_token.string.value);
    if (type_it == map_type_map.end())
        this->throw_error<RDViewParserErrorICF>(type_token, "invalid map type");

    auto horiz_it = border_type_map.find(horiz_token.string.value);
    if (horiz_it == border_type_map.end())
        this->throw_error<RDViewParserErrorICF>(horiz_token, "invalid border type");

    auto vert_it = border_type_map.find(vert_token.string.value);
    if (vert_it == border_type_map.end())
        this->throw_error<RDViewParserErrorICF>(vert_token, "invalid border type");

    RDViewNodeMapBorder *node = this->create_node<RDViewNodeMapBorder>();
    node->map_type              = type_it->second;
    node->horizontal_border_type = horiz_it->second;
    node->vertical_border_type   = vert_it->second;
    return node;
}
