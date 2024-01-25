module;
#include <Eigen/Dense>

#include "AtlasMacros.hpp"

module AllocationNodeModule;

import ExchangeModule;
import ExchangeNodeModule;
import CommissionsModule;


namespace Atlas
{

namespace AST
{

//============================================================================
AllocationNode::~AllocationNode() noexcept
{
}


//============================================================================
AllocationBaseNode::AllocationBaseNode(
	AllocationType m_type,
	Exchange& exchange,
	double epsilon,
	Option<double> alloc_param
) noexcept :
	StrategyBufferOpNode(NodeType::ALLOC, exchange, std::nullopt),
	m_type(m_type),
	m_alloc_param(alloc_param),
	m_epsilon(epsilon)
{
	m_weights_buffer.resize(exchange.getAssetCount());
	m_weights_buffer.setZero();
}


//============================================================================
void
AllocationBaseNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// if we have a commission manager we need to copy the current weights buffer 
	// into the commission manager buffer before it gets overwritten by the ast. 
	// 
	// Additionally if weight epsilon is set we need to copy the current weights
	// so that any adjustments made are reverted back if the absolute value of the
	// differene is less than epsilon
	if (m_commision_manager || m_epsilon)
	{
		m_weights_buffer = target;
	}

	evaluateChild(target);

	// if epsilon is set we need to check if the difference between the current
	// weights and the new weights is less than epsilon. If it is we need to
	// revert the weights back to the original weights before calculating any commissions
	if (m_epsilon)
	{
		target = ((target - m_weights_buffer).cwiseAbs().array() < m_epsilon)
			.select(m_weights_buffer, target);
	}

	// if we have a commission manager we need to calculate the commission caused
	// by the current weights adjustment
	if (m_commision_manager && (*m_commision_manager)->hasCommission())
	{
		(*m_commision_manager)->calculateCommission(target, m_weights_buffer);
	}
}


//============================================================================
AllocationNode::AllocationNode(
	UniquePtr<ExchangeViewNode> exchange_view,
	AllocationType type,
	Option<double> alloc_param,
	double epsilon
) noexcept :
	AllocationBaseNode(
		type,
		exchange_view->getExchange(),
		epsilon,
		alloc_param),
	m_exchange_view(std::move(exchange_view))
{
}


//============================================================================
Result<UniquePtr<AllocationBaseNode>, AtlasException>
	AllocationNode::make(
		UniquePtr<ExchangeViewNode> exchange_view,
		AllocationType type,
		Option<double> alloc_param,
		double epsilon
	) noexcept
{
	if (
		type == AllocationType::CONDITIONAL_SPLIT
		&&
		!alloc_param) {
		return Err("Allocation type requires a parameter");
	}

	return std::make_unique<AllocationNode>(
		std::move(exchange_view), type, alloc_param, epsilon
	);
}


//============================================================================
void
	AllocationNode::evaluateChild(Eigen::VectorXd& target) noexcept
{
	// evaluate the exchange view to calculate the signal
	m_exchange_view->evaluate(target);

	// calculate the number of non-NaN elements in the signal
	size_t nonNanCount = target.unaryExpr([](double x) { return !std::isnan(x) ? 1 : 0; }).sum();
	double c;
	if (nonNanCount > 0) {
		c = (1.0) / static_cast<double>(nonNanCount);
	}
	else {
		c = 0.0;
	}

	switch (m_type) {
	case AllocationType::UNIFORM: {
		target = target.unaryExpr([c](double x) { return x == x ? c : 0.0; });
		break;
	}
	case AllocationType::CONDITIONAL_SPLIT: {
		target = (target.array() < *m_alloc_param)
			.select(-c, (target.array() > *m_alloc_param)
				.select(c, target));
		target = target.unaryExpr([](double x) { return x == x ? x : 0.0; });
		break;
	}
	}
}


//============================================================================
PyNodeWrapper<AllocationBaseNode> AllocationNode::pyMake(
	PyNodeWrapper<ExchangeViewNode> exchange_view,
	AllocationType type,
	Option<double> alloc_param,
	double epsilon)
{
	if (!exchange_view.has_node())
	{
		throw std::runtime_error("exchange view was taken");
	}
	auto node = AllocationNode::make(
		std::move(exchange_view.take()),
		type,
		alloc_param,
		epsilon
	);
	if (node.has_value())
	{
		return PyNodeWrapper<AllocationBaseNode>(std::move(node.value()));
	}
	else
	{
		throw std::runtime_error(node.error().what());
	}
}

//============================================================================
size_t
	AllocationNode::getWarmup() const noexcept
{
	return m_exchange_view->getWarmup();
}


}

}