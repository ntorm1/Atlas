export module AtlasSymbolModule;

import AtlasLangTypes;
import AtlasToken;
import AtlasCore;

namespace Atlas
{


namespace AST
{


//============================================================================
struct Symbol
{
	Token token;
};



//============================================================================
export class SymbolTable
{
private:
	Option<SymbolTable const*> parent = std::nullopt;
	HashMap<String, Symbol> symbols;

public:
	SymbolTable() noexcept = default;
	SymbolTable(SymbolTable const* parent) noexcept : parent(parent) {}
	~SymbolTable() noexcept = default;

};

}


}