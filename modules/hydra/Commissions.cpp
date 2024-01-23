module;
#include <Eigen/Dense>
module CommissionsModule;


import StrategyModule;

namespace Atlas
{


CommisionManager::CommisionManager(Strategy& m_strategy
) noexcept : m_strategy(m_strategy)
{
	size_t alloc_buffer = m_strategy.getAllocationBuffer().size();
	m_buffer.resize(alloc_buffer);
	m_buffer.setZero();
}


//============================================================================
void
CommisionManager::bufferCopy(LinAlg::EigenVectorXd const& target_weights) noexcept
{
	assert(m_buffer.size() == target_weights.size());
	m_buffer = target_weights;
}


//============================================================================
void
CommisionManager::calculateCommission(LinAlg::EigenVectorXd const& target_weights) noexcept
{
	double commission = 0.0;
	double strategy_nlv = m_strategy.getNLV();
	if (m_commission_pct)
	{
		commission += calculatePctCommission(strategy_nlv, target_weights, m_buffer);
	}
	if (m_fixed_commision)
	{
		commission += calculateFixedCommission(target_weights);
	}
	strategy_nlv -= commission;
	m_strategy.setNlv(strategy_nlv);
}


//============================================================================
double
CommisionManager::calculateFixedCommission(
	LinAlg::EigenVectorXd const& target_weights) noexcept
{
	assert(m_fixed_commision);
	assert(target_weights.size() == m_buffer.size());

	// get a count of number of weights that are different
	size_t count = (target_weights - m_buffer)
		.unaryExpr([](double x) { return std::abs(x) < ORDER_EPSILON ? 0 : 1; })
		.sum();

	return count * (*m_fixed_commision);
}


//============================================================================
double
CommisionManager::calculatePctCommission(
	double nlv,
	LinAlg::EigenVectorXd const& target_weights,
	LinAlg::EigenVectorXd const& current_weights) noexcept
{
	assert(m_commission_pct);
	assert(target_weights.size() == current_weights.size());

	// get a count of number of weights that are different
	return ((target_weights - current_weights)
		* nlv * m_commission_pct.value()).sum();
}


}