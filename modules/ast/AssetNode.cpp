module;
#include "AtlasMacros.hpp"
#include <Eigen/Dense>
module AssetNodeModule;

import ExchangeModule;

namespace Atlas
{

namespace AST
{

//============================================================================
size_t
	AssetReadNode::size() const noexcept
{
	return m_exchange.getAssetCount();
}

//============================================================================
void
AssetReadNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
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
AssetReadNode::AssetReadNode(size_t column, int row_offset, Exchange& exchange) noexcept
	: StrategyBufferOpNode(NodeType::ASSET_READ, exchange, std::nullopt),
	m_column(column),
	m_row_offset(row_offset),
	m_exchange(exchange),
	m_warmup(static_cast<size_t>(std::abs(m_row_offset))) 
{}


//============================================================================
Result<SharedPtr<AssetReadNode>, AtlasException>
AssetReadNode::make(
		String const& column,
		int row_offset,
		Exchange& m_exchange
	) noexcept
{
	Option<size_t> column_index = m_exchange.getColumnIndex(column);
	if (!column_index)
	{
		return Err("Column not found");
	}
	return std::make_unique<AssetReadNode>(
		*column_index, row_offset, m_exchange
	);
}


//============================================================================
SharedPtr<AssetReadNode>
AssetReadNode::pyMake(String const& column, int row_offset, Exchange& m_exchange)
{
	auto result = make(column, row_offset, m_exchange);
	if (result)
	{
		return std::move(*result);
	}
	else
	{
		throw std::runtime_error(result.error().what());
	}
}


//============================================================================
AssetOpNode::AssetOpNode(
	SharedPtr<StrategyBufferOpNode> asset_op_left,
	SharedPtr<StrategyBufferOpNode> asset_op_right,
	AssetOpType op_type
) noexcept:
	StrategyBufferOpNode(NodeType::ASSET_OP, asset_op_left->getExchange(), std::nullopt),
	m_asset_op_left(std::move(asset_op_left)),
	m_asset_op_right(std::move(asset_op_right)),
	m_op_type(op_type)
{
	warmup = std::max(m_asset_op_left->getWarmup(), m_asset_op_right->getWarmup());
	m_right_buffer.resize(getExchange().getAssetCount());
	m_right_buffer.setZero();
}


//============================================================================
SharedPtr<AssetOpNode>
AssetOpNode::pyMake(
	SharedPtr<StrategyBufferOpNode> asset_op_left,
	SharedPtr<StrategyBufferOpNode> asset_op_right,
	AssetOpType op_type)
{
	auto res = make(std::move(asset_op_left), std::move(asset_op_right), op_type);
	if (!res)
	{
		throw AtlasException(res.error());
	}
	return std::move(res.value());
}


//============================================================================
void
AssetOpNode::swapLeft(
	SharedPtr<ASTNode> asset_op,
	SharedPtr<StrategyBufferOpNode>& left
) noexcept
{
	auto asset_op_node = std::dynamic_pointer_cast<AssetOpNode>(asset_op);
	std::swap(asset_op_node->getLeft(), left);
}


//============================================================================
void
AssetOpNode::swapRight(
	SharedPtr<ASTNode> asset_op,
	SharedPtr<StrategyBufferOpNode>& right
) noexcept
{
	auto asset_op_node = std::dynamic_pointer_cast<AssetOpNode>(asset_op);
	std::swap(asset_op_node->getRight(), right);
}


//============================================================================
size_t
AssetOpNode::refreshWarmup() noexcept
{
	warmup = std::max(m_asset_op_left->refreshWarmup(), m_asset_op_right->refreshWarmup());
	return warmup;
}


void
AssetOpNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	assert(static_cast<size_t>(target.rows()) == getExchange().getAssetCount());
	assert(static_cast<size_t>(target.cols()) == 1);

	m_asset_op_left->evaluate(target);
	m_asset_op_right->evaluate(m_right_buffer);

	assert(target.size() == m_right_buffer.size());
	switch (m_op_type)
	{
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
AssetMedianNode::AssetMedianNode(
	SharedPtr<Exchange> exchange,
	size_t col_1,
	size_t col_2
) noexcept:
	StrategyBufferOpNode(NodeType::ASSET_MEDIAN, *exchange, std::nullopt),
	m_col_1(col_1),
	m_col_2(col_2)
{
}


//============================================================================
SharedPtr<AssetMedianNode>
AssetMedianNode::pyMake(SharedPtr<Exchange> exchange, String const& col_1, String const& col_2)
{
	auto column_index1 = exchange->getColumnIndex(col_1);
	auto column_index2 = exchange->getColumnIndex(col_2);
	if (!column_index1 || !column_index2)
		throw std::runtime_error("Column not found");
	return AssetMedianNode::make(exchange, *column_index1, *column_index2);
}

//============================================================================
AssetMedianNode::~AssetMedianNode() noexcept
{
}


//============================================================================
void AssetMedianNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept
{
	target = (m_exchange.getSlice(m_col_1, 0) + m_exchange.getSlice(m_col_2, 0)) / 2;
}



//============================================================================
ATRNode::ATRNode(
	Exchange& exchange,
	size_t high,
	size_t low,
	size_t window
) noexcept :
	StrategyBufferOpNode(NodeType::ASSET_ATR, exchange, std::nullopt),
	m_high(high),
	m_low(low),
	m_window(window)
{
	m_atr.resize(
		m_exchange.getAssetCount(),
		m_exchange.getTimestamps().size()
	);
	m_atr.setZero();
	m_close = m_exchange.getCloseIndex().value();
}


//============================================================================
void
ATRNode::build() noexcept
{
	auto const& data = m_exchange.getData();
	size_t col_count = m_exchange.getHeaders().size();

	// True Range holder for EMA
	LinAlg::EigenMatrixXd tr(m_exchange.getAssetCount(), m_window);
	tr.setZero();
	size_t tr_idx = 0;
	double alpha = 1 / static_cast<double>(m_window);

	for (size_t i = 1; i < m_exchange.getTimestamps().size(); ++i)
	{
		size_t high_idx = i * col_count + m_high;
		size_t low_idx = i * col_count + m_low;
		size_t close_idx = i * col_count + m_close;

		auto high_slice = data.col(high_idx);
		auto low_slice = data.col(low_idx);
		auto close_slice = data.col(close_idx);

		auto tr0 = (high_slice - low_slice).cwiseAbs();
		auto tr1 = (high_slice - data.col(close_idx-col_count)).cwiseAbs();
		auto tr2 = (low_slice - data.col(close_idx-col_count)).cwiseAbs();
		tr.col(tr_idx) = tr0.cwiseMax(tr1).cwiseMax(tr2);

		if (i < m_window)
		{
			continue;
		}
		else if (i == m_window - 1)
		{
			m_atr.col(i) = tr.rowwise().mean();
		}
		else if (i > m_window - 1)
		{
			m_atr.col(i) = m_atr.col(i - 1) * (1 - alpha) + tr.col(tr_idx) * alpha;
		}
		++tr_idx;
		tr_idx %= m_window;
	}
}


//============================================================================
SharedPtr<ATRNode>
ATRNode::pyMake(
	SharedPtr<Exchange> exchange,
	String const& high,
	String const& low,
	size_t window
)
{
	auto high_idx = exchange->getColumnIndex(high);
	auto low_idx = exchange->getColumnIndex(low);
	if (!high_idx || !low_idx) {
		throw std::runtime_error("Invalid column name");
	}

	auto node = ATRNode::make(*exchange, *high_idx, *low_idx, window);
	auto same = exchange->getSameFromCache(node);
	if (same)
	{
		return std::dynamic_pointer_cast<ATRNode>(*same);
	}
	node->build();
	return node;
}


//============================================================================
bool
ATRNode::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	if (other->getType() != NodeType::ASSET_ATR)
	{
		return false;
	}
	auto other_atr = static_cast<ATRNode*>(other.get());
	return m_high == other_atr->getHigh() &&
		m_low == other_atr->getLow() &&
		m_window == other_atr->getWindow();
}

//============================================================================
ATRNode::~ATRNode() noexcept
{
}


//============================================================================
void
ATRNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	target = m_atr.col(m_exchange.currentIdx());
}

}

}