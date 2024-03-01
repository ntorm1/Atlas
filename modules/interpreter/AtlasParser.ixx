module;

export module AtlasParserModule;

import AtlasLangTypes;
import AtlasToken;
import AtlasCore;

namespace Atlas
{


namespace AST
{


//============================================================================
export class Parser
{
private:
	UniquePtr<Lexer> m_lexer;
	UniquePtr<SymbolTable> m_symbol_table;
	Set<String> m_type_names;
	Token const* m_current_token= nullptr;
	size_t m_current_token_index = 0;

	Token const* getNextToken() noexcept;
	void handleIdentifier() noexcept;
	void handleFn() noexcept;

	[[nodiscard]] AtlasResult<UniquePtr<FunctionAST>> parseFn() noexcept;
	[[nodiscard]] AtlasResult<UniquePtr<PrototypeAST>> parsePrototype() noexcept;

public:
	Parser() noexcept;
	~Parser() noexcept;

	Result<bool, AtlasException> loadSource(const String& source) noexcept;
	Result<bool, AtlasException> buildAST() noexcept;

};



}

}