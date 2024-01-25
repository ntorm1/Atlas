module;
export module BaseNodeModule;

import AtlasCore;

namespace Atlas
{

namespace AST
{

//============================================================================
export enum class NodeType
{
	ASSET_READ = 0,
	ASSET_OP = 1,
	EXCHANGE_VIEW = 2,
	ALLOC = 3,
	STRATEGY = 4,
	STRATEGY_RUNNER = 5,
	ALLOC_WEIGHT = 6,
	RANK_NODE = 7,
	TRADE_LIMIT = 8
};


//============================================================================
export enum class AssetOpType
{
	ADD = 0,
	SUBTRACT = 1,
	MULTIPLY = 2,
	DIVIDE = 3,
};


//============================================================================
export class ASTNode {
public:
	ASTNode(
		NodeType type,
		Option<ASTNode*> parent = std::nullopt
	) : _type(type), _parent(parent) {}
	
	virtual ~ASTNode() {}
	virtual size_t getWarmup() const noexcept = 0;
	
	NodeType getType() const noexcept { return _type; }
	Option<ASTNode*> getParent() const noexcept { return _parent; }

private:
	Option<ASTNode*> _parent = std::nullopt;
	NodeType _type;
};


//============================================================================
export template <typename T>
class ExpressionNode : public ASTNode {
public:
	ExpressionNode(NodeType type) : ASTNode(type) {}
	virtual ~ExpressionNode() {}
	virtual T evaluate() noexcept = 0;
};


//============================================================================
export template <typename Result, typename... Params>
class OpperationNode : public ASTNode {
public:
	OpperationNode(
		NodeType type,
		Option<ASTNode*> parent = std::nullopt
	) : ASTNode(type, parent) {}
	virtual ~OpperationNode() {}
	virtual Result evaluate(Params...) noexcept = 0;
};


//============================================================================
export class StatementNode : public ASTNode {
public:
	StatementNode(NodeType type) : ASTNode(type) {}
	virtual ~StatementNode() {}
	virtual void evaluate() = 0;
};


}

}