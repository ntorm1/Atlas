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
LinAlg::EigenConstColView<double>
	AssetReadNode::evaluate() noexcept
{
	return m_exchange.getSlice(m_column, m_row_offset);
}


//============================================================================
PyNodeWrapper<AssetReadNode>
AssetReadNode::pyMake(String const& column, int row_offset, SharedPtr<Exchange> exchange)
{
	auto node = make(column, row_offset, *exchange);
	if (!node)
	{
		throw std::exception(node.error().what());
	}
	return PyNodeWrapper<AssetReadNode>(std::move(*node));
}


//============================================================================
Result<UniquePtr<AssetReadNode>, AtlasException>
	AssetReadNode::make(
		String const& column,
		int row_offset,
		Exchange const& m_exchange
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

}

}