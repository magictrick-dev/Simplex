#include <rdview/rdtokenizer.hpp>

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

void RDViewTokenizer::
format_token_as(RDViewToken *token, RDViewTokenType type)
{
    token->type                     = type;
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