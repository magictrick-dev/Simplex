#include <rdview/rdtokenizer.hpp>

enum RDViewDFAState
{
    RDViewDFAState_WhitespaceEntry,
    RDViewDFAState_WhitespaceSingle,
    RDViewDFAState_WhitespaceCommentEntry,
    RDViewDFAState_WhitespaceCommentBody,
    RDViewDFAState_WhitespaceExit,
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

}

bool RDViewTokenizer:: 
previous_token_is(RDViewTokenType type) const
{
    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer:: 
current_token_is(RDViewTokenType type) const
{
    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer:: 
next_token_is(RDViewTokenType type) const
{
    SIMPLEX_NO_IMPLEMENTATION();
}

void RDViewTokenizer::
format_token_as(RDViewToken *token, RDViewTokenType type)
{
    token->type     = type;
    token->line     = this->line;
    token->column   = this->column;
    token->length   = this->offset - this->step;
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

bool RDViewTokenizer::
handle_numbers(RDViewToken *token)
{

    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer::
handle_strings(RDViewToken *token)
{

    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer::
handle_identifiers(RDViewToken *token)
{

    SIMPLEX_NO_IMPLEMENTATION();
}

RDViewIdentifierType RDViewTokenizer::
classify_identifier(RDViewToken *token)
{

    SIMPLEX_NO_IMPLEMENTATION();
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
    bool result = (this->step >= this->source.length());
    return result;
}