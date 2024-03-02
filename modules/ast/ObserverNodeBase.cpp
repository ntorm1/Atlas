module;

#include <Eigen/Dense>

module ObserverNodeModule;

import ExchangeModule;
import AssetNodeModule;

namespace Atlas
{

namespace AST
{

//============================================================================
AssetObserverNode::AssetObserverNode(
	String const& id,
	SharedPtr<StrategyBufferOpNode> parent,
	AssetObserverType observer_type,
	size_t window
) noexcept :
	StrategyBufferOpNode(NodeType::ASSET_OBSERVER, parent->getExchange(), parent.get()),
	m_parent(parent),
	m_window(window),
	m_warmup(window),
	m_id(id),
	m_observer_warmup(window),
	m_observer_type(observer_type)
{
	m_buffer_matrix.resize(m_exchange.getAssetCount(), window);
	m_buffer_matrix.setZero();
}


//============================================================================
AssetObserverNode::~AssetObserverNode() noexcept
{
}


//============================================================================
void
	AssetObserverNode::resetBase() noexcept
{
	m_buffer_matrix.setZero();
	m_buffer_idx = 0;
	reset();
}


//============================================================================
void
AssetObserverNode::cacheBase() noexcept
{
	if (m_exchange.currentIdx() < m_observer_warmup)
	{
		m_buffer_idx = (m_buffer_idx + 1) % m_window;
		return;
	}
	auto buffer_ref = buffer();
	m_parent->evaluate(buffer_ref);
	cacheObserver();
	if (m_exchange.currentIdx() >= (m_window - 1))
	{
		m_buffer_idx = (m_buffer_idx + 1) % m_window;
		onOutOfRange(m_buffer_matrix.col(m_buffer_idx));
	}
}




//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
AssetObserverNode::buffer() noexcept
{
	size_t col = 0;
	assert(m_buffer_idx < static_cast<size_t>(m_buffer_matrix.cols()));
	return m_buffer_matrix.col(m_buffer_idx);
}


//============================================================================
void
AssetObserverNode::enableSignalCopy() noexcept
{
	m_signal_copy.resize(m_exchange.getAssetCount());
	m_signal_copy.setConstant(-std::numeric_limits<double>::max());
}



}


}