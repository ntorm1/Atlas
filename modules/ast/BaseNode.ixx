module;
export module BaseNodeModule;

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
class ASTNode {
public:
	ASTNode(NodeType type) : _type(type) {}
	virtual ~ASTNode() {}
	virtual size_t getWarmup() const noexcept = 0;
private:
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
export template <typename Result, typename Param = void>
class OpperationNode : public ASTNode {
public:
	OpperationNode(NodeType type) : ASTNode(type) {}
	virtual ~OpperationNode() {}
	virtual Result evaluate(Param) noexcept = 0;
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