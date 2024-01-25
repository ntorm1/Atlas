module;
#include <Eigen/Dense>
#include <iostream>
#include "AtlasMacros.hpp"
module StrategyNodeModule;

import ExchangeModule;

import HelperNodesModule;
import ExchangeNodeModule;
import RiskNodeModule;
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
size_t
AllocationNode::getWarmup() const noexcept
{
	return m_exchange_view->getWarmup();
}


//============================================================================
StrategyNode::StrategyNode(
	UniquePtr<AllocationBaseNode> allocation,
	Portfolio& portfolio
) noexcept :
	OpperationNode<bool, Eigen::VectorXd&>(NodeType::STRATEGY, allocation.get()),
	m_allocation(std::move(allocation)),
	m_portfolio(portfolio)
{
	m_warmup = m_allocation->getWarmup();
}


//============================================================================
StrategyNode::~StrategyNode() noexcept
{
}


//============================================================================
bool
StrategyNode::evaluate(Eigen::VectorXd& target) noexcept
{
	if (m_trigger && !(*m_trigger)->evaluate())
	{
		return false;
	}
	m_allocation->evaluate(target);
	if (m_alloc_weight)
	{
		(*m_alloc_weight)->evaluate(target);
	}
	return true;
}


//============================================================================
void
StrategyNode::setCommissionManager(SharedPtr<CommisionManager> manager) noexcept
{
	// search the strategy node call chain for the allocation node so 
	// we can attach commission manager to it. If we don't find it then
	// that means there is no allocation node, return false to indicate error.
	m_allocation->setCommissionManager(manager);
}


//============================================================================s
Exchange&
StrategyNode::getExchange() noexcept
{
	return m_allocation->getExchange();
}


//============================================================================
void
StrategyNode::reset() noexcept
{
	if (m_trigger)
	{
		(*m_trigger)->reset();
	}
}

//============================================================================
void
StrategyNode::setTrigger(SharedPtr<TriggerNode> trigger) noexcept
{
	m_trigger = trigger;
}


//============================================================================
FixedAllocationNode::~FixedAllocationNode() noexcept
{

}


//============================================================================
FixedAllocationNode::FixedAllocationNode(
	Vector<std::pair<size_t, double>> m_allocations,
	Exchange* exchange,
	double epsilon
) noexcept :
	AllocationBaseNode(
		AllocationType::FIXED,
		*exchange,
		epsilon,
		std::nullopt),
	m_allocations(std::move(m_allocations))
{
	m_allocations = std::move(m_allocations);
}


//============================================================================
Result<UniquePtr<AllocationBaseNode>, AtlasException>
FixedAllocationNode::make(
	Vector<std::pair<String, double>> allocations,
	Exchange* exchange,
	double epsilon
) noexcept
{
	Vector<std::pair<size_t, double>> allocations_ids;
	for (const auto& pair : allocations)
	{
		auto id_opt = exchange->getAssetIndex(pair.first);
		if (!id_opt)
		{
			return Err("Asset not found: " + pair.first);
		}
		allocations_ids.push_back(std::make_pair(*id_opt, pair.second));
	}
	return std::make_unique<FixedAllocationNode>(
		std::move(allocations_ids), exchange, epsilon
	);
}


//============================================================================
UniquePtr<AllocationBaseNode>
FixedAllocationNode::pyMake(
	Vector<std::pair<String, double>> m_allocations,
	SharedPtr<Exchange> exchange,
	double epsilon
)
{
	auto res = make(std::move(m_allocations), exchange.get(), epsilon);
	if (!res)
	{
		throw std::exception(res.error().what());
	}
	return std::move(*res);
}


//============================================================================
void
FixedAllocationNode::evaluateChild(Eigen::VectorXd& target) noexcept
{
	for (auto& pair : m_allocations)
	{
		assert(pair.first < static_cast<size_t>(target.size()));
		target(pair.first) = pair.second;
	}
}



}

}