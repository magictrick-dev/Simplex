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
}

RDViewTokenizer::
~RDViewTokenizer()
{

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