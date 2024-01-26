module;
#include <Eigen/Dense>
#include <iostream>
#include "AtlasMacros.hpp"
module StrategyNodeModule;

import ExchangeModule;

import HelperNodesModule;
import ExchangeNodeModule;
import RiskNodeModule;

namespace Atlas
{

namespace AST
{

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
PyNodeWrapper<StrategyNode> StrategyNode::pyMake(
	PyNodeWrapper<AllocationBaseNode> allocation,
	Portfolio& portfolio)
{
	if (!allocation.has_node())
	{
		throw std::runtime_error("allocation was taken");
	}
	auto node = StrategyNode::make(
		std::move(allocation.take()),
		portfolio
	);
	return PyNodeWrapper<StrategyNode>(std::move(node));
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
	m_allocation->reset();
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
PyNodeWrapper<AllocationBaseNode>
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
	auto node = std::move(*res);
	return PyNodeWrapper<AllocationBaseNode>(std::move(node));
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