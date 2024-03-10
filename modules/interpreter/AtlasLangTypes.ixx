module;

export module AtlasLangTypes;

import AtlasCore;

namespace Atlas
{


namespace AST
{
	export class Lexer;
	export class Parser;
	export class Token;

	export class FunctionAST;
	export class PrototypeAST;

	export template <typename T>
	using AtlasResult = Result<T, AtlasException>;

	export enum TokenType
	{
		tok_eof = -1,

		// primary
		tok_quotation = 1,
		tok_colon = 2,
		tok_identifier = 3,
		tok_number = 4,
		tok_open_paren = 5,
		tok_close_paren = 6,
		tok_bin_op = 8,

		// control
		tok_if = 11,
		tok_then = 12,
		tok_else = 13,
		tok_return = 15,
		tok_comma = 17,
		tok_var = 18,
	};

}

}