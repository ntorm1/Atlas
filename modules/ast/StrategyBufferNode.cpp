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
	if (v && m_cache.cols() != cols)
	{
		m_cache.resize(rows, cols);
		m_cache.setZero();
	}
	else if (!v && m_cache.cols() > 1)
	{
		m_cache.resize(0, 0);
	}
}


//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
StrategyBufferOpNode::cacheColumn(Option<size_t> col) noexcept
{
	if (col)
	{
		assert(col.value() < static_cast<size_t>(m_cache.cols()));
		return m_cache.col(col.value());
	}
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


//============================================================================
SharedPtr<StrategyBufferOpNode>
StrategyBufferOpNode::lag(size_t lag) noexcept
{
	enableCache(true);
	auto lag_node = std::make_shared<LagNode>(this, lag);
	return lag_node;
}

//============================================================================
Option<Vector<double>>
StrategyBufferOpNode::getAssetCacheSlice(size_t asset_index) const noexcept
{
	if (asset_index >= static_cast<size_t>(m_cache.rows()))
	{
		return std::nullopt;
	}
	if (m_cache.cols() <= 1)
	{
		return std::nullopt;
	}
	Vector<double> slice(m_cache.cols());
	for (int i = 0; i < m_cache.cols(); ++i)
	{
		slice.push_back(m_cache(asset_index, i));
	}
	return std::move(slice);
}


//============================================================================
size_t
StrategyBufferOpNode::getAssetCount() const noexcept
{
	return m_exchange.getAssetCount();
}


//============================================================================
LagNode::LagNode(
	StrategyBufferOpNode* parent,
	size_t lag
) noexcept :
	StrategyBufferOpNode(NodeType::LAG, parent->getExchange(), parent),
	m_lag(lag),
	m_parent(parent)
{
	assert(parent->cache().cols() > 1);
}



//============================================================================
void
LagNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	size_t col_idx = m_exchange.currentIdx();
	if (col_idx < m_lag)
	{
		target.setConstant(std::numeric_limits<float>::quiet_NaN());
		return;
	}
	assert(target.rows() == m_parent->cache().rows());
	target = m_parent->cache().col(col_idx - m_lag);
}

//============================================================================
size_t
LagNode::getWarmup() const noexcept
{
	return m_parent->getWarmup() + m_lag;
}


//============================================================================
bool
LagNode::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	if (other->getType() != NodeType::LAG)
	{
		return false;
	}
	auto other_lag = static_cast<LagNode*>(other.get());
	return m_lag == other_lag->m_lag;
}


}

}