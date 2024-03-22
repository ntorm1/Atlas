module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module AssetLogicalModule;

namespace Atlas {

namespace AST {

//============================================================================
AssetIfNode::AssetIfNode(SharedPtr<StrategyBufferOpNode> left_eval,
                         AssetCompType comp_type,
                         SharedPtr<StrategyBufferOpNode> right_eval) noexcept
    : StrategyBufferOpNode(
          NodeType::ASSET_IF, left_eval->getExchange(),
          Vector<SharedPtr<StrategyBufferOpNode>>({left_eval, right_eval})),
      m_left_eval(left_eval), m_right_eval(right_eval), m_comp_type(comp_type) {
  m_warmup = std::max({left_eval->getWarmup(), right_eval->getWarmup()});
  m_buffer.resize(m_left_eval->getAssetCount());
  m_buffer.setZero();
  left_eval->addChild(this);
  right_eval->addChild(this);
}

//============================================================================
AssetIfNode::~AssetIfNode() noexcept {}

//============================================================================
bool AssetIfNode::isSame(StrategyBufferOpNode const* other) const noexcept {
  if (other->getType() != NodeType::ASSET_IF)
    return false;
  auto ptr = static_cast<AssetIfNode const*>(other);
  return m_left_eval->isSame(ptr->m_left_eval.get()) &&
         m_right_eval->isSame(ptr->m_right_eval.get()) &&
         m_comp_type == ptr->m_comp_type;
}

//============================================================================
void AssetIfNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  m_left_eval->evaluate(target);
  m_right_eval->evaluate(m_buffer);

  switch (m_comp_type) {
  case AssetCompType::EQUAL:
    target = (target.array() == m_buffer.array()).cast<double>();
    break;
  case AssetCompType::NOT_EQUAL:
    target = (target.array() != m_buffer.array()).cast<double>();
    break;
  case AssetCompType::GREATER:
    target = (target.array() > m_buffer.array()).cast<double>();
    break;
  case AssetCompType::GREATER_EQUAL:
    target = (target.array() >= m_buffer.array()).cast<double>();
    break;
  case AssetCompType::LESS:
    target = (target.array() < m_buffer.array()).cast<double>();
    break;
  case AssetCompType::LESS_EQUAL:
    target = (target.array() <= m_buffer.array()).cast<double>();
    break;
  }

  if (hasCache())
    cacheColumn() = target;
}

//============================================================================
void AssetIfNode::reset() noexcept {
  m_left_eval->reset();
  m_right_eval->reset();
  m_buffer.setZero();
}

//============================================================================
void AssetCompNode::reset() noexcept {
  m_left_eval->reset();
  m_right_eval->reset();
  m_true_eval->reset();
  m_false_eval->reset();
  m_buffer.setZero();
}

//============================================================================
AssetCompNode::AssetCompNode(
    SharedPtr<StrategyBufferOpNode> left_eval, LogicalType logicial_type,
    SharedPtr<StrategyBufferOpNode> right_eval,
    SharedPtr<StrategyBufferOpNode> true_eval,
    SharedPtr<StrategyBufferOpNode> false_eval) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_COMP, left_eval->getExchange(),
                           Vector<SharedPtr<StrategyBufferOpNode>>(
                               {left_eval, right_eval, true_eval, false_eval})),
      m_left_eval(left_eval), m_right_eval(right_eval), m_true_eval(true_eval),
      m_false_eval(false_eval), m_logical_type(logicial_type) {
  m_warmup = std::max({left_eval->getWarmup(), right_eval->getWarmup(),
                       true_eval->getWarmup(), false_eval->getWarmup()});
  m_buffer.resize(m_left_eval->getAssetCount(), 3);
  m_buffer.setZero();
  left_eval->addChild(this);
  right_eval->addChild(this);
  true_eval->addChild(this);
  false_eval->addChild(this);
}

//============================================================================
AssetCompNode::~AssetCompNode() noexcept {}

//============================================================================
void AssetCompNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  m_left_eval->evaluate(target);
  m_right_eval->evaluate(m_buffer.col(RIGHT_EVAL_IDX));
  m_true_eval->evaluate(m_buffer.col(TRUE_EVAL_IDX));
  m_false_eval->evaluate(m_buffer.col(FALSE_EVAL_IDX));
  switch (m_logical_type) {
  case LogicalType::AND:
    target =
        ((target.array() != 0) && (m_buffer.col(RIGHT_EVAL_IDX).array() != 0))
            .select(m_buffer.col(TRUE_EVAL_IDX), m_buffer.col(FALSE_EVAL_IDX));
  case LogicalType::OR:
    target =
        ((target.array() != 0) || (m_buffer.col(RIGHT_EVAL_IDX).array() != 0))
            .select(m_buffer.col(TRUE_EVAL_IDX), m_buffer.col(FALSE_EVAL_IDX));
    break;
  }
  if (hasCache())
    cacheColumn() = target;
}

//============================================================================
bool AssetCompNode::isSame(
    StrategyBufferOpNode const* other) const noexcept {
  if (other->getType() != NodeType::ASSET_COMP)
    return false;
  auto ptr = static_cast<AssetCompNode const*>(other);
  return m_left_eval->isSame(ptr->getLeftEval().get()) &&
         m_right_eval->isSame(ptr->getRightEval().get()) &&
         m_true_eval->isSame(ptr->getTrueEval().get()) &&
         m_false_eval->isSame(ptr->getFalseEval().get()) &&
         m_logical_type == ptr->getLogicalType();
}

//============================================================================
void AssetIfNode::swapRightEval(
    SharedPtr<StrategyBufferOpNode> right_eval) noexcept {
  m_right_eval = right_eval;
  m_warmup = std::max(m_warmup, m_right_eval->getWarmup());
}

//============================================================================
void AssetIfNode::swapLeftEval(
    SharedPtr<StrategyBufferOpNode> left_eval) noexcept {
  m_left_eval = left_eval;
  m_warmup = std::max(m_warmup, m_left_eval->getWarmup());
}

//============================================================================
void AssetCompNode::swapFalseEval(
    SharedPtr<StrategyBufferOpNode> false_eval) noexcept {
  m_false_eval = false_eval;
  m_warmup = std::max(m_warmup, m_false_eval->getWarmup());
}

//============================================================================
void AssetCompNode::swapTrueEval(
    SharedPtr<StrategyBufferOpNode> true_eval) noexcept {
  m_true_eval = true_eval;
  m_warmup = std::max(m_warmup, m_true_eval->getWarmup());
}

} // namespace AST

} // namespace Atlas