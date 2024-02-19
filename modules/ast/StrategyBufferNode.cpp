module;
#include <Eigen/Dense>
module StrategyBufferModule;

import AllocationNodeModule;
import ExchangeModule;

namespace Atlas
{

namespace AST
{

	//============================================================================
Option<AllocationBaseNode*>
StrategyBufferOpNode::getAllocationNode() const noexcept {
	Option<ASTNode*> m_parent = getParent();
	while (m_parent) {
		if (m_parent.value()->getType() == NodeType::ALLOC) {
			return static_cast<AllocationBaseNode*>(*m_parent);
		}
		m_parent = m_parent.value()->getParent();
	}
	return std::nullopt;
}


//============================================================================
void
StrategyBufferOpNode::setTakeFromCache(bool v) noexcept
{
	if (v && m_cache.cols() > 1)
	{
		m_take_from_cache = true;
	}
	else
	{
		m_take_from_cache = false;
	}
}


//============================================================================
void
StrategyBufferOpNode::enableCache(bool v) noexcept
{
	size_t rows = m_exchange.getAssetCount();
	size_t cols = m_exchange.getTimestamps().size();
	if (v)
	{
		m_cache.resize(rows, cols);
		m_cache.setZero();
	}
	else 
	{
		m_cache.resize(0, 0);
	}
}


//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
StrategyBufferOpNode::cacheColumn() noexcept
{
	if (m_cache.cols() > 1)
	{
		size_t col_idx = m_exchange.currentIdx();
		return m_cache.col(col_idx);
	}
	if (m_cache.cols() == 0)
	{
		size_t rows = m_exchange.getAssetCount();
		m_cache.resize(rows, 1);
		m_cache.setZero();
		return m_cache.col(0);
	}
	return m_cache.col(0);
}


}

}