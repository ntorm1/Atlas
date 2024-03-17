module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module AssetNodeModule;

import ExchangeModule;

namespace Atlas {

namespace AST {

//============================================================================
AssetReadNode::AssetReadNode(size_t column, int row_offset,
                             Exchange &exchange) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_READ, exchange, std::nullopt),
      m_column(column), m_row_offset(row_offset),
      m_warmup(static_cast<size_t>(std::abs(m_row_offset))) {}

//============================================================================
size_t AssetReadNode::size() const noexcept {
  return m_exchange.getAssetCount();
}

//============================================================================
bool AssetReadNode::isSame(
    SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_READ) {
    return false;
  }
  auto node = static_cast<AssetReadNode *>(other.get());
  return m_column == node->getColumn() && m_row_offset == node->getRowOffset();
}

//============================================================================
void AssetReadNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  auto slice = m_exchange.getSlice(m_column, m_row_offset);
  assert(static_cast<size_t>(slice.rows()) == m_exchange.getAssetCount());
  size_t slice_rows = static_cast<size_t>(slice.rows());
  size_t target_rows = static_cast<size_t>(target.rows());
  assert(slice_rows == target_rows);
  assert(static_cast<size_t>(slice.cols()) == 1);
  assert(static_cast<size_t>(target.cols()) == 1);
  target = slice;
}

//============================================================================
Result<SharedPtr<AssetReadNode>, AtlasException>
AssetReadNode::make(String const &column, int row_offset,
                    Exchange &m_exchange) noexcept {
  Option<size_t> column_index = m_exchange.getColumnIndex(column);
  if (!column_index) {
    return Err("Column not found");
  }
  return std::make_unique<AssetReadNode>(*column_index, row_offset, m_exchange);
}

//============================================================================
SharedPtr<AssetReadNode> AssetReadNode::pyMake(String const &column,
                                               int row_offset,
                                               Exchange &m_exchange) {
  auto result = make(column, row_offset, m_exchange);
  if (result) {
    return std::move(*result);
  } else {
    throw std::runtime_error(result.error().what());
  }
}

//============================================================================
AssetOpNode::AssetOpNode(SharedPtr<StrategyBufferOpNode> asset_op_left,
                         SharedPtr<StrategyBufferOpNode> asset_op_right,
                         AssetOpType op_type) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_OP, asset_op_left->getExchange(),
                           Vector<SharedPtr<StrategyBufferOpNode>>(
                               {asset_op_left, asset_op_right})),
      m_asset_op_left(std::move(asset_op_left)),
      m_asset_op_right(std::move(asset_op_right)), m_op_type(op_type) {
  warmup =
      std::max(m_asset_op_left->getWarmup(), m_asset_op_right->getWarmup());
  m_right_buffer.resize(getExchange().getAssetCount());
  m_right_buffer.setZero();
  m_asset_op_left->addChild(this);
  m_asset_op_right->addChild(this);
}

//============================================================================
SharedPtr<AssetOpNode>
AssetOpNode::pyMake(SharedPtr<StrategyBufferOpNode> asset_op_left,
                    SharedPtr<StrategyBufferOpNode> asset_op_right,
                    AssetOpType op_type) {
  auto res = make(std::move(asset_op_left), std::move(asset_op_right), op_type);
  if (!res) {
    throw AtlasException(res.error());
  }
  return std::move(res.value());
}

//============================================================================
void AssetOpNode::swapLeft(SharedPtr<ASTNode> asset_op,
                           SharedPtr<StrategyBufferOpNode> &left) noexcept {
  auto asset_op_node = std::dynamic_pointer_cast<AssetOpNode>(asset_op);
  std::swap(asset_op_node->getLeft(), left);
}

//============================================================================
void AssetOpNode::swapRight(SharedPtr<ASTNode> asset_op,
                            SharedPtr<StrategyBufferOpNode> &right) noexcept {
  auto asset_op_node = std::dynamic_pointer_cast<AssetOpNode>(asset_op);
  std::swap(asset_op_node->getRight(), right);
}

//============================================================================
size_t AssetOpNode::refreshWarmup() noexcept {
  warmup = std::max(m_asset_op_left->refreshWarmup(),
                    m_asset_op_right->refreshWarmup());
  return warmup;
}

//============================================================================
bool AssetOpNode::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_OP) {
    return false;
  }
  auto other_asset_op = static_cast<AssetOpNode *>(other.get());
  return m_op_type == other_asset_op->getOpType() &&
         m_asset_op_left->isSame(other_asset_op->getLeft()) &&
         m_asset_op_right->isSame(other_asset_op->getRight());
}

//============================================================================
void AssetOpNode::reset() noexcept {
  m_asset_op_left->reset();
  m_asset_op_right->reset();
  m_right_buffer.setZero();
}

//============================================================================
void AssetOpNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
#ifdef _DEBUG
  size_t target_rows = static_cast<size_t>(target.rows());
  size_t asset_count = getExchange().getAssetCount();
  assert(target_rows == asset_count);
  assert(static_cast<size_t>(target.cols()) == 1);
#endif

  m_asset_op_left->evaluate(target);
  m_asset_op_right->evaluate(m_right_buffer);

  assert(target.size() == m_right_buffer.size());
  switch (m_op_type) {
  case AssetOpType::ADD:
    target = target + m_right_buffer;
    break;
  case AssetOpType::SUBTRACT:
    target = target - m_right_buffer;
    break;
  case AssetOpType::MULTIPLY:
    target = target.cwiseProduct(m_right_buffer);
    break;
  case AssetOpType::DIVIDE:
    target = target.cwiseQuotient(m_right_buffer);
    break;
  }
}

//============================================================================
AssetMedianNode::AssetMedianNode(SharedPtr<Exchange> exchange, size_t col_1,
                                 size_t col_2) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_MEDIAN, *exchange, std::nullopt),
      m_col_1(col_1), m_col_2(col_2) {}

//============================================================================
SharedPtr<AssetMedianNode> AssetMedianNode::pyMake(SharedPtr<Exchange> exchange,
                                                   String const &col_1,
                                                   String const &col_2) {
  auto column_index1 = exchange->getColumnIndex(col_1);
  auto column_index2 = exchange->getColumnIndex(col_2);
  if (!column_index1 || !column_index2)
    throw std::runtime_error("Column not found");
  return AssetMedianNode::make(exchange, *column_index1, *column_index2);
}

//============================================================================
AssetMedianNode::~AssetMedianNode() noexcept {}

//============================================================================
bool AssetMedianNode::isSame(
    SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_MEDIAN) {
    return false;
  }
  auto other_median = static_cast<AssetMedianNode *>(other.get());
  return m_col_1 == other_median->getCol1() &&
         m_col_2 == other_median->getCol2();
}

//============================================================================
void AssetMedianNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  target =
      (m_exchange.getSlice(m_col_1, 0) + m_exchange.getSlice(m_col_2, 0)) / 2;
}

//============================================================================
ATRNode::ATRNode(Exchange &exchange, size_t high, size_t low,
                 size_t window) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_ATR, exchange, std::nullopt),
      m_high(high), m_low(low), m_window(window) {
  m_close = m_exchange.getCloseIndex().value();
}

//============================================================================
void ATRNode::build() noexcept {
  if (m_window > m_exchange.getTimestamps().size() || m_window == 0) {
    return;
  }

  enableCache();
  auto const &data = m_exchange.getData();
  size_t col_count = m_exchange.getHeaders().size();

  size_t tr_idx = 0;
  double alpha = 1 / static_cast<double>(m_window);
  size_t timestamp_count = m_exchange.getTimestamps().size();
  size_t asset_count = m_exchange.getAssetCount();
  Eigen::VectorXd tr0 = Eigen::VectorXd::Zero(asset_count);
  Eigen::VectorXd tr1 = Eigen::VectorXd::Zero(asset_count);
  Eigen::VectorXd tr2 = Eigen::VectorXd::Zero(asset_count);
  for (size_t i = 0; i < timestamp_count; ++i) {
    size_t high_idx = i * col_count + m_high;
    size_t low_idx = i * col_count + m_low;
    size_t close_idx = i * col_count + m_close;

    if (i == 0) {
      cacheColumn(i) = (data.col(high_idx) - data.col(low_idx)).cwiseAbs();
      continue;
    }

    tr0 = (data.col(high_idx) - data.col(low_idx)).cwiseAbs();
    tr1 = (data.col(high_idx) - data.col(close_idx - col_count)).cwiseAbs();
    tr2 = (data.col(low_idx) - data.col(close_idx - col_count)).cwiseAbs();
    cacheColumn(i) = alpha * tr0.cwiseMax(tr1).cwiseMax(tr2) +
                     (1 - alpha) * cacheColumn(i - 1);
  }
}

//============================================================================
SharedPtr<ATRNode> ATRNode::pyMake(SharedPtr<Exchange> exchange,
                                   String const &high, String const &low,
                                   size_t window) {
  auto high_idx = exchange->getColumnIndex(high);
  auto low_idx = exchange->getColumnIndex(low);
  if (!high_idx || !low_idx) {
    throw std::runtime_error("Invalid column name");
  }

  auto node = ATRNode::make(*exchange, *high_idx, *low_idx, window);
  auto same = exchange->getSameFromCache(node);
  if (same) {
    return std::dynamic_pointer_cast<ATRNode>(*same);
  }
  node->build();
  return node;
}

//============================================================================
bool ATRNode::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_ATR) {
    return false;
  }
  auto other_atr = static_cast<ATRNode *>(other.get());
  return m_high == other_atr->getHigh() && m_low == other_atr->getLow() &&
         m_window == other_atr->getWindow();
}

//============================================================================
ATRNode::~ATRNode() noexcept {}

//============================================================================
void ATRNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  target = cacheColumn(m_exchange.currentIdx());
}

//============================================================================
AssetScalerNode::AssetScalerNode(SharedPtr<StrategyBufferOpNode> parent,
                                 AssetOpType op_type, double scale) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_SCALAR, parent->getExchange(),
                           parent.get()),
      m_op_type(op_type), m_scale(scale), m_parent(parent) {
  parent->addChild(this);
}

//============================================================================
AssetScalerNode::~AssetScalerNode() noexcept {}

//============================================================================
void AssetScalerNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  m_parent->evaluate(target);
  switch (m_op_type) {
  case AssetOpType::ADD:
    target.array() += m_scale;
    break;
  case AssetOpType::SUBTRACT:
    target.array() -= m_scale;
    break;
  case AssetOpType::MULTIPLY:
    target.array() *= m_scale;
    break;
  case AssetOpType::DIVIDE:
    target.array() /= m_scale;
    break;
  }
}

//============================================================================
void AssetScalerNode::reset() noexcept { m_parent->reset(); }

//============================================================================
bool AssetScalerNode::isSame(
    SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_SCALAR) {
    return false;
  }
  auto other_scaler = static_cast<AssetScalerNode *>(other.get());
  return m_op_type == other_scaler->getOpType() &&
         m_scale == other_scaler->getScale() &&
         m_parent->isSame(other_scaler->getParent());
}

//============================================================================
AssetFunctionNode::AssetFunctionNode(SharedPtr<StrategyBufferOpNode> parent,
                                     AssetFunctionType func_type,
                                     Option<double> func_param) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_FUNCTION, parent->getExchange(),
                           parent.get()),
      m_func_type(func_type), m_parent(parent), m_func_param(func_param) {
  parent->addChild(this);
}

//============================================================================
AssetFunctionNode::~AssetFunctionNode() noexcept {}

//============================================================================
void AssetFunctionNode::reset() noexcept { m_parent->reset(); }

//============================================================================
void AssetFunctionNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  m_parent->evaluate(target);
  switch (m_func_type) {
  case AssetFunctionType::ABS:
    target = target.array().abs();
    break;
  case AssetFunctionType::SIGN:
    target = target.array().sign();
    break;
  case AssetFunctionType::POWER:
    assert(m_func_param);
    target = target.array().pow(m_func_param.value());
    break;
  case AssetFunctionType::LOG:
    target = target.array().log();
    break;
  }
  if (hasCache())
    cacheColumn() = target;
}

//============================================================================
bool AssetFunctionNode::isSame(
    SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_FUNCTION) {
    return false;
  }
  auto other_func = static_cast<AssetFunctionNode *>(other.get());
  return m_func_type == other_func->getFuncType() &&
         m_func_param == other_func->getFuncParam() &&
         m_parent->isSame(other_func->getParent());
}

} // namespace AST

} // namespace Atlas