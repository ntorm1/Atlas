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
SumObserverNode::SumObserverNode(
	Option<String> id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept:
	AssetObserverNode(id, parent, AssetObserverType::SUM, window)
{
	m_sum.resize(m_exchange.getAssetCount());
	m_sum.setZero();
	setObserverWarmup(parent->getWarmup());
	setWarmup(parent->getWarmup() + window - 1);
}



//============================================================================
SumObserverNode::~SumObserverNode() noexcept
{
}


//============================================================================
void
SumObserverNode::cacheObserver() noexcept
{
	m_sum += buffer();
	if (hasCache() && m_exchange.currentIdx() >= (getWindow() - 1))
		cacheColumn() = m_sum;
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
	if (m_signal_copy.size())
		m_signal_copy = m_sum;
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
	Option<String> id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(id, parent, AssetObserverType::MEAN, window)
{
	Option<String> sum_id = std::nullopt;
	if (id.has_value())
	{
		sum_id = id.value() + "_sum_";
	}
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
	Option<String> id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(id, parent, AssetObserverType::MAX, window)
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
	if (m_signal_copy.size() > 0)
		m_signal_copy = m_max;
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
	m_max = buffer().cwiseMax(m_max);
	if (hasCache())
		cacheColumn() = m_max;
}


//============================================================================
void
MaxObserverNode::reset() noexcept
{
	m_max.setConstant(-std::numeric_limits<double>::max());
	setObserverBuffer(-std::numeric_limits<double>::max());
}


//============================================================================
void
TsArgMaxObserverNode::reset() noexcept
{
	m_arg_max.setConstant(-std::numeric_limits<double>::max());
	setObserverBuffer(-std::numeric_limits<double>::max());
}


//============================================================================
TsArgMaxObserverNode::TsArgMaxObserverNode(
	Option<String> id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept :
	AssetObserverNode(id, parent, AssetObserverType::TS_ARGMAX, window)
{
	Option<String> max_id = std::nullopt;
	if (id.has_value())
	{
		max_id = id.value() + "_sum_";
	}
	auto max = std::make_shared<MaxObserverNode>(max_id, parent, window);
	m_max_observer = std::dynamic_pointer_cast<MaxObserverNode>(m_exchange.registerObserver(std::move(max)));
	m_max_observer->enableSignalCopy();
	setObserverWarmup(parent->getWarmup());
	setWarmup(parent->getWarmup() + window);

	m_arg_max.resize(m_exchange.getAssetCount());
	m_arg_max.setConstant(0);
	setObserverBuffer(0);
}





//============================================================================
TsArgMaxObserverNode::~TsArgMaxObserverNode() noexcept
{
}


//============================================================================
void
TsArgMaxObserverNode::cacheObserver() noexcept
{
	auto const& buffer_matrix = m_max_observer->getBufferMatrix();
	auto v = static_cast<double>(getWindow());
	m_arg_max = (
		buffer_matrix.col(getBufferIdx()).array() == m_max_observer->getSignalCopy().array()
		).select(v, m_arg_max.array()-1);
	if (hasCache())
		cacheColumn() = m_arg_max;
}


//============================================================================
void
TsArgMaxObserverNode::onOutOfRange(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old
) noexcept
{
	size_t buffer_idx = getBufferIdx();
	size_t window = getWindow();
	auto const& buffer_matrix = m_max_observer->getBufferMatrix();
	size_t columns = static_cast<size_t>(buffer_matrix.cols());
	for (size_t i = 0; i < static_cast<size_t>(m_arg_max.rows()); i++)
	{
		auto const& row = buffer_matrix.row(i);
		// Column going out of range is the argmax if the value is equal to the window, 
		// otherwise it is not the argmax and we can skip it
		auto old = m_arg_max(i);
		if (old > 1)
		{
			continue;
		}

		// get the max coefficient from the buffer row
		double max = -std::numeric_limits<double>::max();
		size_t max_idx = 0;
		for (size_t j = 0; j < columns; j++)
		{
			if (row(j) > max)
			{
				max = row(j);
				max_idx = j;
			}
		}
		max_idx = (max_idx - buffer_idx + window) % window;
		max_idx += 1;
		m_arg_max(i) = static_cast<double>(max_idx);
		cacheColumn()(i) = m_arg_max(i);
	}
}


//============================================================================
void
TsArgMaxObserverNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept
{
	target = m_arg_max;
}


//============================================================================
VarianceObserverNode::VarianceObserverNode(
	Option<String> id,
	SharedPtr<StrategyBufferOpNode> parent,
	size_t window
) noexcept:
	AssetObserverNode(id, parent, AssetObserverType::VARIANCE, window)
{
	Option<String> sum_id = std::nullopt;
	if (id.has_value())
		sum_id = id.value() + "_mean";
	auto mean = std::make_shared<MeanObserverNode>(sum_id, parent, window);
	m_mean_observer = std::dynamic_pointer_cast<MeanObserverNode>(m_exchange.registerObserver(std::move(mean)));
	m_mean_observer->enableSignalCopy();

	Option<String> sum_sq_id = std::nullopt;
	if (id.has_value())
		sum_sq_id = id.value() + "_sum_sq";
	auto squared_node = std::make_shared<AssetFunctionNode>(
		parent,
		AssetFunctionType::POWER,
		2.0f
	);
	auto sum_sq = std::make_shared<SumObserverNode>(
		std::move(sum_sq_id),
		std::move(squared_node),
		window
	);
	m_sum_squared_observer = std::dynamic_pointer_cast<SumObserverNode>(m_exchange.registerObserver(std::move(sum_sq)));
	m_sum_squared_observer->enableSignalCopy();
	m_variance.resize(m_exchange.getAssetCount());
	m_variance.setConstant(0);
	m_sum_squared_cache.resize(m_exchange.getAssetCount());
	m_sum_squared_cache.setConstant(0);
	setObserverWarmup(parent->getWarmup());
	setWarmup(parent->getWarmup() + window);
}


//============================================================================
VarianceObserverNode::~VarianceObserverNode() noexcept
{
}


//============================================================================
void
VarianceObserverNode::onOutOfRange(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old
) noexcept
{
}


//============================================================================
void 
VarianceObserverNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept
{
	target = m_variance;
}


//============================================================================
void
VarianceObserverNode::cacheObserver() noexcept
{
	m_mean_observer->evaluate(m_variance);
	m_sum_squared_observer->evaluate(m_sum_squared_cache);
	m_variance = m_sum_squared_cache - (getWindow() * m_variance.cwiseProduct(m_variance));
	if (getWindow() > 1)
		m_variance /= static_cast<double>(getWindow() - 1);
	if (hasCache())
		cacheColumn() = m_variance;
}


//============================================================================
void
VarianceObserverNode::reset() noexcept
{
	m_variance.setConstant(0);
	m_sum_squared_cache.setConstant(0);
}


}

}