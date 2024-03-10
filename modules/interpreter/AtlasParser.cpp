module;
#include "AtlasMacros.hpp"
module AtlasParserModule;

import AtlasLexerModule;
import AtlasFunctionModule;

namespace Atlas
{


namespace AST
{

//============================================================================
Parser::Parser() noexcept
{
	m_lexer = std::make_unique<Lexer>();
}


//============================================================================
Token const*
Parser::getNextToken() noexcept
{
	auto const& tokens = m_lexer->getTokens();
	m_current_token = &tokens[m_current_token_index];
	++m_current_token_index;
	return m_current_token;
}


//============================================================================
Parser::~Parser() noexcept
{
}


//============================================================================
Result<bool, AtlasException>
Parser::loadSource(const String& source) noexcept
{
	m_lexer->tokenize(source);
	return true;
}


//============================================================================
void
Parser::handleIdentifier() noexcept
{
	auto const& idName = m_current_token->getStr();
	getNextToken(); // consume the identifier
}


//============================================================================
AtlasResult<UniquePtr<PrototypeAST>>
Parser::parsePrototype() noexcept
{
	return Err("not implemented");
}



//============================================================================
Result<bool, AtlasException>
Parser::buildAST() noexcept
{
	if (!m_lexer->getTokens().size()) 
	{
		return Err("No tokens to parse");
	}
	auto const& tokens = m_lexer->getTokens();
	while (m_current_token_index < tokens.size())
	{
		auto& token = tokens[m_current_token_index++];
		m_current_token = &token;
		switch (token.getType())
		{
			case tok_eof:
				return true;
			case tok_identifier:
				handleIdentifier();
				break;
		}
	}

	return true;
}

}


}