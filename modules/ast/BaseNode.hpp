#pragma once
#include "standard/AtlasCore.hpp"

namespace Atlas {

namespace AST {

//============================================================================
enum class NodeType  {
  ALLOC = 0,
  ALLOC_WEIGHT = 1,
  ASSET_OP = 2,
  ASSET_COMP = 3,
  ASSET_IF = 4,
  ASSET_FUNCTION = 5,
  ASSET_MEDIAN = 6,
  ASSET_READ = 7,
  ASSET_OBSERVER = 8,
  ASSET_SCALAR = 9,
  ASSET_ATR = 10,
  COVARIANCE = 11,
  EXCHANGE_VIEW = 12,
  RANK_NODE = 13,
  STRATEGY = 14,
  STRATEGY_RUNNER = 15,
  TRADE_LIMIT = 16,
  MODEL = 17,
  LAG = 18,
  TARGET = 19,
  NOP = 20,
  ASSET_PCA = 21,
  CLUSTER = 22,
};

//============================================================================
enum class AssetOpType  {
  ADD = 0,
  SUBTRACT = 1,
  MULTIPLY = 2,
  DIVIDE = 3,
};

//============================================================================
enum class AssetFunctionType  {
  SIGN = 0,
  POWER = 1,
  ABS = 3,
  LOG = 4
};

//============================================================================
enum class AssetCompType  {
  EQUAL = 0,
  NOT_EQUAL = 1,
  GREATER = 2,
  LESS = 3,
  GREATER_EQUAL = 4,
  LESS_EQUAL = 5,
};

//============================================================================
class ASTNode {
public:
  ASTNode(NodeType type, Option<ASTNode *> parent = std::nullopt) noexcept
      : m_type(type) {
    if (parent.has_value()) {
      m_parent.push_back(parent.value());
		}
  }
  ASTNode(NodeType type,
          Vector<SharedPtr<StrategyBufferOpNode>> parent) noexcept;
  ASTNode(const ASTNode &) = delete;

  virtual ~ASTNode() {}
  virtual size_t getWarmup() const noexcept = 0;
  virtual void reset() noexcept = 0;

  NodeType getType() const noexcept { return m_type; }
  auto const &getParents() const noexcept { return m_parent; }
  Option<ASTNode *> getParent() const noexcept {
    if (m_parent.size() > 0) {
			return m_parent[0];
		}
    return std::nullopt;
  }

private:
  Vector<ASTNode *> m_parent;
  NodeType m_type;
};

//============================================================================
template <typename T> class ExpressionNode : public ASTNode {
public:
  ExpressionNode(NodeType type) : ASTNode(type) {}
  virtual ~ExpressionNode() {}
  virtual T evaluate() noexcept = 0;
};

//============================================================================
template <typename Result, typename... Params>
class OpperationNode : public ASTNode {
public:
  OpperationNode(NodeType type, Option<ASTNode *> parent = std::nullopt) noexcept
      : ASTNode(type, parent) {}
  OpperationNode(NodeType type,
                 Vector<SharedPtr<StrategyBufferOpNode>> parent) noexcept
			: ASTNode(type, std::move(parent)) {}

  virtual ~OpperationNode() {}
  virtual Result evaluate(Params...) noexcept = 0;
};

//============================================================================
class StatementNode : public ASTNode {
public:
  StatementNode(NodeType type) : ASTNode(type) {}
  virtual ~StatementNode() {}
  virtual void evaluate() noexcept = 0;
};

} // namespace AST

} // namespace Atlas