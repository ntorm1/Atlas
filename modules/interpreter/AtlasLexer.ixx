module;

export module AtlasLexerModule;

import AtlasLangTypes;
import AtlasToken;
import AtlasCore;

namespace Atlas
{


namespace AST
{

//============================================================================
export class Lexer
{
private:
	String m_source;
	char last_char = '\0';
	const char* m_current = nullptr;
	Token* current_token = nullptr;
	String identifier_str = "";
	Vector<Token> m_tokens; 

	char getNextChar() noexcept;
	char peekNextChar() noexcept;
	Token getNextToken() noexcept;

public:
	Lexer() noexcept;
	~Lexer() noexcept;

	auto const& getTokens() const noexcept { return m_tokens; }
	Result<bool, AtlasException> tokenize(String const& source) noexcept;
		
};


}

}