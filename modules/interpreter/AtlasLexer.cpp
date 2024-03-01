module;
#include "AtlasMacros.hpp"
#include <cassert>
#include <cctype>
#include <filesystem>
module AtlasLexerModule;


namespace Atlas
{

namespace AST
{

//============================================================================
char
Lexer::getNextChar() noexcept
{
	assert(*m_current != '\0');
	++m_current;
	return *m_current;
}


//============================================================================
char
Lexer::peekNextChar() noexcept
{
	assert(*m_current != '\0');
	return *(m_current + 1);
}


//============================================================================
Token
Lexer::getNextToken() noexcept
{
    // Skip any whitespace.
    while (isspace(last_char)) {
        last_char = getNextChar();
    }

    if (isalpha(last_char)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        identifier_str = last_char;
        Uint32 size = 1;
        while (isalnum((last_char = getNextChar()))) {
            identifier_str += last_char;
            size++;
        }

        if (identifier_str == "if")
            return Token(tok_if, m_current - size, size);
        if (identifier_str == "fn")
            return Token(tok_fn, m_current - size, size);
        if (identifier_str == "then")
            return Token(tok_then, m_current - size, size);
        if (identifier_str == "else")
            return Token(tok_else, m_current - size, size);
        return Token(tok_identifier, m_current - size, size);
    }

    if (isdigit(last_char) || last_char == '.') { // Number: [0-9.]+
        String NumStr;
        Uint32 size = 0;
        do {
            NumStr += last_char;
            last_char = getNextChar();
            size++;
        } while (isdigit(last_char) || last_char == '.');
        return Token(tok_number, m_current - size, size);
    }

    if (last_char == '#') {
        // Comment until end of line.
        do
            last_char = getNextChar();
        while (last_char != '\n' && last_char != '\r');

        if (last_char != -1)
            return getNextToken();
    }

    // Check for end of file.  Don't eat the EOF.
    if (last_char == '\0')
        return Token(tok_eof, m_current, 0);

    const char* pos = this->m_current;
    if (last_char == '(') {
        last_char = getNextChar();
        return Token(tok_open_paren, pos, 1);
    }
    if (last_char == ')') {
        last_char = getNextChar();
        return Token(tok_close_paren, pos, 1);
    }
    if (last_char == '{') {
        last_char = getNextChar();
        return Token(tok_open_brace, pos, 1);
    }
    if (last_char == '}') {
        last_char = getNextChar();
        return Token(tok_close_brace, pos, 1);
    }
    if (last_char == ';') {
        last_char = getNextChar();
        return Token(tok_semicolon, pos, 1);
    }
    if (last_char == ':') {
        last_char = getNextChar();
        return Token(tok_colon, pos, 1);
    }
    if (last_char == '=') {
        last_char = getNextChar();
        return Token(tok_equal, pos, 1);
    }
    if (last_char == ',') {
        last_char = getNextChar();
        return Token(tok_comma, pos, 1);
    }
    if (last_char == '-')
    {
        if (peekNextChar() == '>') {
            last_char = getNextChar();
            last_char = getNextChar();
            return Token(tok_arrow, pos, 2);
        }
    }
    if (last_char == '+' || last_char == '-' || last_char == '*' || last_char == '/') {
        last_char = getNextChar();
        return Token(tok_bin_op, pos, 1);
    }
    assert(false && "Unknown token");
    std::unreachable();
}


//============================================================================
Lexer::Lexer() noexcept
{

}


//============================================================================
Lexer::~Lexer() noexcept
{
}


//============================================================================
Result<bool, AtlasException>
Lexer::tokenize(String const& source) noexcept
{
    if (source.empty()) {
        return Err("No source to parse");
    }
    do {
        m_tokens.push_back(getNextToken());
        current_token = &m_tokens.back();
    } while (
        current_token->getType() != tok_eof
        &&
        (*m_current) != '\0'
        );
    return true;

}


}

}