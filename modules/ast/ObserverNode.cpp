module;

#include <Eigen/Dense>

module ObserverNodeModule;

import ExchangeModule;

namespace Atlas
{

namespace AST
{

	//============================================================================
AssetObserverNode::AssetObserverNode(
	Exchange& exchange,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	StrategyBufferOpNode(NodeType::ASSET_OBSERVER, exchange, parent.get()),
	m_parent(parent),
	m_window(window)
{
	m_buffer_matrix.resize(exchange.getAssetCount(), window);
	m_buffer_matrix.setZero();
}


//============================================================================
AssetObserverNode::~AssetObserverNode() noexcept
{
}


//============================================================================
void
AssetObserverNode::cacheBase() noexcept
{
	cache();
	size_t current_index = m_exchange.currentIdx();
	if (current_index >= m_window)
	{
		auto col = m_buffer_matrix.col(m_buffer_idx);
		onOutOfRange(col);
	}
	m_buffer_idx++;
	if (m_buffer_idx == m_window)
	{
		m_buffer_idx = 0;
	}
}

//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
AssetObserverNode::buffer() noexcept
{
	return m_buffer_matrix.col(m_buffer_idx);
}


//============================================================================
SumObserverNode::SumObserverNode(
	SharedPtr<Exchange> exchange,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept:
	AssetObserverNode(*exchange, parent, window),
	m_window(window)
{
	m_sum.resize(m_exchange.getAssetCount());
	m_sum.setZero();
}


//============================================================================
SumObserverNode::~SumObserverNode() noexcept
{
}


//============================================================================
void
SumObserverNode::cache() noexcept
{
	auto buffer_ref = buffer();
	m_parent->evaluate(buffer_ref);
	m_sum += buffer_ref;
}


//============================================================================
void
SumObserverNode::onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept
{
	m_sum -= buffer_old;
}

//============================================================================
void
SumObserverNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept 
{
	target = m_sum;
}


//============================================================================
AssetScalerNode::AssetScalerNode(
	SharedPtr<StrategyBufferOpNode> parent,
	AssetOpType op_type,
	double scale
) noexcept :
	StrategyBufferOpNode(NodeType::ASSET_SCALAR, parent->getExchange(), parent.get()),
	m_op_type(op_type),
	m_scale(scale)
{
}


//============================================================================
AssetScalerNode::~AssetScalerNode() noexcept
{
}

//============================================================================
void
AssetScalerNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	switch (m_op_type)
	{
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


}

}