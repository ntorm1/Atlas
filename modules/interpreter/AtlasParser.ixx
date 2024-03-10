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
	Set<String> m_type_names;
	Token const* m_current_token= nullptr;
	size_t m_current_token_index = 0;

	Token const* getNextToken() noexcept;
	void handleIdentifier() noexcept;

	[[nodiscard]] AtlasResult<UniquePtr<PrototypeAST>> parsePrototype() noexcept;

public:
	Parser() noexcept;
	~Parser() noexcept;

	Result<bool, AtlasException> loadSource(const String& source) noexcept;
	Result<bool, AtlasException> buildAST() noexcept;

};



}

}