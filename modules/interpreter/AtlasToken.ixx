export module AtlasToken;

import AtlasCore;
import AtlasLangTypes;

namespace Atlas
{

namespace AST
{


//============================================================================
export class Token
{
private:
	TokenType m_type;
	const char* PtrData;
	Uint32 len;

public:
	Token(TokenType type, const char* data, Uint32 len)
		: m_type(type), PtrData(data), len(len)
	{}

	TokenType getType() const { return m_type; }
	const char* getData() const { return PtrData; }
	Uint32 getLength() const { return len; };

	StringRef getStrRef() const { return StringRef(PtrData, len); }
	String getStr() const { return String(PtrData, len); }
};


}

}