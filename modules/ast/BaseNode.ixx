module;
export module BaseNodeModule;

import AtlasCore;

namespace Atlas
{

namespace AST
{

//============================================================================
export enum class NodeType : Uint8
{
	ALLOC = 0,
	ALLOC_WEIGHT = 1,
	ASSET_OP = 2,
	ASSET_READ = 3,
	ASSET_OBSERVER = 4,
	ASSET_SCALAR = 5,
	COVARIANCE = 6,
	EXCHANGE_VIEW = 7,
	RANK_NODE = 8,
	STRATEGY = 9,
	STRATEGY_RUNNER = 10,
	TRADE_LIMIT = 11,
};


//============================================================================
export enum class AssetOpType : Uint8
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
	virtual void evaluate() noexcept = 0;
};


}

}