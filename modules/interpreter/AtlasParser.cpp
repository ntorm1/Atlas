module;
#include "AtlasMacros.hpp"
module AtlasParserModule;

import AtlasLexerModule;
import AtlasFunctionModule;
import AtlasSymbolModule;

namespace Atlas
{


namespace AST
{

//============================================================================
Parser::Parser() noexcept
{
	m_lexer = std::make_unique<Lexer>();
	m_symbol_table = std::make_unique<SymbolTable>();
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
void
Parser::handleFn() noexcept
{
	auto res = parseFn();
}


//============================================================================
AtlasResult<UniquePtr<FunctionAST>>
Parser::parseFn() noexcept
{
	getNextToken(); // eat fn token
	auto Proto = parsePrototype();


	return Err("not implemented");
}


//============================================================================
AtlasResult<UniquePtr<PrototypeAST>>
Parser::parsePrototype() noexcept
{
	if (m_current_token->getType() != tok_identifier) {
		return Err("Expected function name in prototype");
	}
	auto const& FnName = m_current_token->getStr();
	getNextToken(); // eat function name

	if (m_current_token->getType() != tok_open_paren) {
		return Err("Expected '(' in prototype");
	}

	// Read the list of argument names.
	Vector<std::pair<String, String>> arg_names;
	Vector<String> arg_types;
	while (getNextToken()->getType() == tok_identifier) {
		String arg = m_current_token->getStr();
		if (getNextToken()->getType() != tok_colon) {
			return Err("Expected ':'");
		}
		if (getNextToken()->getType() != tok_identifier) {
			return Err("Expected identifier");
		}
		String type = m_current_token->getStr();
		arg_names.push_back(std::make_pair(arg, type));
	}

	if (getNextToken()->getType() != tok_close_paren) {
		return Err("Expected ')'");
	}

	// extract return type of function 
	if (getNextToken()->getType() != tok_arrow) {
		return Err("Expected '->' in prototype");
	}
	if (getNextToken()->getType() != tok_identifier) {
		return Err("Expected identifier");
	}
	String returnType = m_current_token->getStr();
	return std::make_unique<PrototypeAST>(FnName, arg_names, returnType);
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
			case tok_fn:
				handleFn();
				break;
			case tok_identifier:
				handleIdentifier();
				break;
		}
	}

	return true;
}

}


}