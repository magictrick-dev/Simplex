#include <rdview/rdtokenizer.hpp>

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

}

RDViewTokenizer::
~RDViewTokenizer()
{

}

RDViewToken RDViewTokenizer:: 
get_previous_token() const
{
    SIMPLEX_NO_IMPLEMENTATION();
}

RDViewToken RDViewTokenizer:: 
get_current_token() const
{
    SIMPLEX_NO_IMPLEMENTATION();
}

RDViewToken RDViewTokenizer:: 
get_next_token() const
{
    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer:: 
shift()
{
    SIMPLEX_NO_IMPLEMENTATION();
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
consume_whitespace()
{

    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer::
handle_numbers()
{

    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer::
handle_strings()
{

    SIMPLEX_NO_IMPLEMENTATION();
}

bool RDViewTokenizer::
handle_identifiers()
{

    SIMPLEX_NO_IMPLEMENTATION();
}

RDViewIdentifierType RDViewTokenizer::
classify_identifier()
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