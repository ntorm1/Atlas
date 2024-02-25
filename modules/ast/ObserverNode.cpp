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
	m_warmup(window),
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
	cacheObserver();
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
	AssetObserverNode(parent, AssetObserverType::SUM, window)
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
SumObserverNode::cacheObserver() noexcept
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
	size_t hash_value = 17; // Initialize with a prime number

	// Combine hash with member variables
	hash_value = hash_value * 31 + std::hash<Uint32>{}(static_cast<Uint32>(getWindow()));
	hash_value = hash_value * 31 + type_hash;

	// Mix the bits to increase entropy
	hash_value ^= (hash_value >> 16);
	hash_value *= 0x85ebca6b;
	hash_value ^= (hash_value >> 13);
	hash_value *= 0xc2b2ae35;
	hash_value ^= (hash_value >> 16);

	return hash_value;
}

//============================================================================
size_t MeanObserverNode::hash() const noexcept
{
	Uint8 type_hash = static_cast<Uint8>(m_observer_type);
	size_t hash_value = 17; // Initialize with a prime number

	// Combine hash with member variables
	hash_value = hash_value * 31 + std::hash<Uint32>{}(static_cast<Uint32>(getWindow()));
	hash_value = hash_value * 31 + type_hash;

	// Mix the bits to increase entropy
	hash_value ^= (hash_value >> 16);
	hash_value *= 0x85ebca6b;
	hash_value ^= (hash_value >> 13);
	hash_value *= 0xc2b2ae35;
	hash_value ^= (hash_value >> 16);

	return hash_value;
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


//============================================================================
MeanObserverNode::MeanObserverNode(
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(parent, AssetObserverType::MEAN, window)
{
	auto sum = std::make_shared<SumObserverNode>(parent, window);
	m_sum_observer = std::dynamic_pointer_cast<SumObserverNode>(m_exchange.registerObserver(std::move(sum)));
	m_scaler = std::make_shared<AssetScalerNode>(
		m_sum_observer,
		AssetOpType::DIVIDE,
		static_cast<double>(window)
	);
}


//============================================================================
MeanObserverNode::~MeanObserverNode() noexcept
{
}


//============================================================================
void
MeanObserverNode::onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept
{
	m_sum_observer->onOutOfRange(buffer_old);
}


//============================================================================
void
MeanObserverNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	m_scaler->evaluate(target);
}

//============================================================================
void
MeanObserverNode::cacheObserver() noexcept
{
	m_sum_observer->cacheObserver();
}


//============================================================================
void
MeanObserverNode::reset() noexcept
{
	m_sum_observer->reset();
}


//============================================================================
size_t 
MeanObserverNode::refreshWarmup() noexcept
{
	setWarmup(m_sum_observer->refreshWarmup());
	return getWarmup();
}


//============================================================================
MaxObserverNode::MaxObserverNode(
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(parent, AssetObserverType::MAX, window)
{
	m_max.resize(m_exchange.getAssetCount());
	m_max.setZero();
}

MaxObserverNode::~MaxObserverNode() noexcept
{
}


}

}