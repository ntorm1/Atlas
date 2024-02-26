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
	if (m_exchange.currentIdx() < m_observer_warmup)
	{
		m_buffer_idx = (m_buffer_idx + 1) % m_window;
		return;
	}

	cacheObserver();
	if (m_exchange.currentIdx() >= (m_window-1))
	{
		if (m_parent->getType() != NodeType::ASSET_READ) {
			onOutOfRange(m_buffer_matrix.col(m_buffer_idx));
			m_buffer_idx = (m_buffer_idx + 1) % m_window;
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
	String id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept:
	AssetObserverNode(std::move(id),parent, AssetObserverType::SUM, window)
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
MeanObserverNode::MeanObserverNode(
	String id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(std::move(id), parent, AssetObserverType::MEAN, window)
{
	auto sum_id = id + "_sum";
	auto sum = std::make_shared<SumObserverNode>(sum_id, parent, window);
	m_sum_observer = std::dynamic_pointer_cast<SumObserverNode>(m_exchange.registerObserver(std::move(sum)));
	m_scaler = std::make_shared<AssetScalerNode>(
		m_sum_observer,
		AssetOpType::DIVIDE,
		static_cast<double>(window)
	);
}


//============================================================================
MaxObserverNode::MaxObserverNode(
	String id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(std::move(id),parent, AssetObserverType::MAX, window)
{
	setObserverWarmup(parent->getWarmup());
	setWarmup(parent->getWarmup() + window);
	m_max.resize(m_exchange.getAssetCount());
	m_max.setConstant(-std::numeric_limits<double>::max());
	setObserverBuffer(-std::numeric_limits<double>::max());
}


//============================================================================
MaxObserverNode::~MaxObserverNode() noexcept
{
}


//============================================================================
void
MaxObserverNode::onOutOfRange(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old
) noexcept
{
	assert(buffer_old.size() == m_max.size());
	size_t buffer_idx = getBufferIdx();
	for (size_t i = 0; i < static_cast<size_t>(m_max.rows()); i++)
	{
		// Check if the column going out of range holds the max value for the row
		if (buffer_old(i) != m_max(i))
		{
			continue;
		}
		
		// loop over the buffer row and find the new max, skipping the expiring index
		auto const& buffer_matrix = getBufferMatrix();
		size_t columns = static_cast<size_t>(buffer_matrix.cols());
		double max = -std::numeric_limits<double>::max();
		for (size_t j = 0; j < columns; j++)
		{
			if (buffer_matrix(i, j) > max && j != buffer_idx)
			{
				max = buffer_matrix(i, j);
			}
		}
		m_max(i) = max;
	}	
}


//============================================================================
void
MaxObserverNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept
{
	assert(target.size() == m_max.size());
	target = m_max;
}


//============================================================================
void
MaxObserverNode::cacheObserver() noexcept
{
	auto buffer_ref = buffer();
	m_parent->evaluate(buffer_ref);
	assert(buffer_ref.size() == m_max.size());
	m_max = buffer_ref.cwiseMax(m_max);
	if (hasCache())
	{
		cacheColumn() = m_max;
	}
}


//============================================================================
void
MaxObserverNode::reset() noexcept
{
	m_max.setConstant(-std::numeric_limits<double>::max());
	setObserverBuffer(-std::numeric_limits<double>::max());
}


//============================================================================
TsArgMaxObserverNode::TsArgMaxObserverNode(
	String id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept:
	AssetObserverNode(std::move(id), parent, AssetObserverType::TS_ARGMAX, window)
{
	setObserverWarmup(parent->getWarmup());
	setWarmup(parent->getWarmup() + window);
	m_arg_max.resize(m_exchange.getAssetCount());
	m_arg_max.setConstant(-std::numeric_limits<double>::max());
	setObserverBuffer(-std::numeric_limits<double>::max());
}


//============================================================================
TsArgMaxObserverNode::~TsArgMaxObserverNode() noexcept
{
}


//============================================================================
void
TsArgMaxObserverNode::onOutOfRange(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old
) noexcept
{
}


//============================================================================
void
TsArgMaxObserverNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept
{
}


//============================================================================
void
TsArgMaxObserverNode::cacheObserver() noexcept
{
}


//============================================================================
void
TsArgMaxObserverNode::reset() noexcept
{
}


}

}