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
	m_null_count = m_exchange.getNullCount(m_row_offset);
	return m_exchange.getSlice(m_column, m_row_offset);
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