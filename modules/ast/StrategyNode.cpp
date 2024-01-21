module;
#include <Eigen/Dense>
#include <iostream>
#include "AtlasMacros.hpp"
module StrategyNodeModule;

import ExchangeModule;

import HelperNodesModule;
import ExchangeNodeModule;

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
	OpperationNode<void, Eigen::VectorXd&>(NodeType::ALLOC),
	m_type(m_type),
	m_alloc_param(alloc_param),
	m_epsilon(epsilon),
	m_exchange(exchange)
{

}


//============================================================================
Exchange&
AllocationBaseNode::getExchange() noexcept
{
	return m_exchange;
}


//============================================================================
AllocationNode::AllocationNode(
	SharedPtr<ExchangeViewNode> exchange_view,
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
Result<SharedPtr<AllocationBaseNode>, AtlasException>
AllocationNode::make(
		SharedPtr<ExchangeViewNode> exchange_view,
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
AllocationNode::evaluate(Eigen::VectorXd& target) noexcept
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
	SharedPtr<AllocationBaseNode> allocation,
	Portfolio& portfolio
) noexcept :
	OpperationNode<bool, Eigen::VectorXd&>(NodeType::STRATEGY),
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
	return true;
}


//============================================================================s
Exchange&
StrategyNode::getExchange() noexcept
{
	return m_allocation->getExchange();
}


//============================================================================
double
StrategyNode::getAllocEpsilon() const noexcept
{
	return m_allocation->getAllocEpsilon();
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
Result<SharedPtr<AllocationBaseNode>, AtlasException>
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
	return std::make_shared<FixedAllocationNode>(
		std::move(allocations_ids), exchange, epsilon
	);
}


//============================================================================
SharedPtr<AllocationBaseNode>
FixedAllocationNode::pyMake(Vector<std::pair<String, double>> m_allocations, SharedPtr<Exchange> exchange, double epsilon)
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
FixedAllocationNode::evaluate(Eigen::VectorXd& target) noexcept
{
	for (auto& pair : m_allocations)
	{
		target(pair.first) = pair.second;
	}
}



}

}