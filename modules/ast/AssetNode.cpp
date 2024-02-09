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
AssetReadNode::evaluate(LinAlg::EigenVectorXd& target) noexcept
{
	target = m_exchange.getSlice(m_column, m_row_offset);
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


void
AssetOpNode::evaluate(LinAlg::EigenVectorXd& target) noexcept
{
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

}

}