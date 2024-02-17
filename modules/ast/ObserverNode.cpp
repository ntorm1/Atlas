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
	SharedPtr<StrategyBufferOpNode> parent,
	AssetObserverType observer_type,
	size_t window
) noexcept :
	StrategyBufferOpNode(NodeType::ASSET_OBSERVER, parent->getExchange(), parent.get()),
	m_parent(parent),
	m_window(window),
	m_observer_type(observer_type)
{
	if (parent->getType() != NodeType::ASSET_READ) {
		m_buffer_matrix.resize(m_exchange.getAssetCount(), window);
	}
	else {
		m_buffer_matrix.resize(m_exchange.getAssetCount(), 1);

	}
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
	cache();
	if (m_exchange.currentIdx() >= m_window)
	{
		if (m_parent->getType() != NodeType::ASSET_READ) {
			onOutOfRange(m_buffer_matrix.col(m_buffer_idx));
			m_buffer_idx++;
			if ((m_buffer_idx % m_window) == 0)
			{
				m_buffer_idx = 0;
			}
		}
		else {
			auto node = static_cast<AssetReadNode*>(m_parent.get());
			int row_offset = -1 * static_cast<int>(m_window);
			auto slice = m_exchange.getSlice(node->getColumn(), row_offset);
			onOutOfRange(slice);
		}
	}
}

//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
AssetObserverNode::buffer() noexcept
{
	size_t col = 0;
	if (m_parent->getType() != NodeType::ASSET_READ) {
		if (m_buffer_idx == 0)
		{
			col = m_window - 1;
		}
		else
		{
			col = m_buffer_idx - 1;
		}
	}
	assert(col < static_cast<size_t>(m_buffer_matrix.cols()));
	return m_buffer_matrix.col(col);
}


//============================================================================
SumObserverNode::SumObserverNode(
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept:
	AssetObserverNode(parent, AssetObserverType::SUM, window),
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
SumObserverNode::reset() noexcept
{
	m_sum.setZero();
}


//============================================================================
size_t SumObserverNode::hash() const noexcept
{
	Uint8 type_hash = static_cast<Uint8>(m_observer_type);
	return m_window ^ (type_hash + 0x9e3779b9 + (m_window << 6) + (m_window >> 2));
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
	m_scale(scale),
	m_parent(parent)
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
	m_parent->evaluate(target);
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