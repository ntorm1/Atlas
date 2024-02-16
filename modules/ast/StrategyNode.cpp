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
	SharedPtr<AllocationBaseNode> allocation,
	Portfolio& portfolio
) noexcept :
	OpperationNode<bool, Eigen::Ref<Eigen::VectorXd>>(NodeType::STRATEGY, allocation.get()),
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
StrategyNode::evaluate(Eigen::Ref<Eigen::VectorXd> target) noexcept
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


//============================================================================
void
StrategyNode::enableCopyWeightsBuffer() noexcept
{
	m_allocation->setCopyWeightsBuffer(true);
}


//============================================================================
Option<SharedPtr<TradeLimitNode>>
StrategyNode::getTradeLimitNode() const noexcept
{
	return m_allocation->getTradeLimitNode();
}


//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
StrategyNode::getPnL() noexcept
{
	return m_allocation->getPnL();
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
StrategyNode::setTracer(SharedPtr<Tracer> tracer) noexcept
{
	m_allocation->setTracer(tracer);
}


//============================================================================
SharedPtr<StrategyNode> StrategyNode::make(
	SharedPtr<AllocationBaseNode> allocation,
	Portfolio& portfolio
)
{
	// when we make a strategy node we need to make sure that the allocation node
	// has not been used on the side by another strategy node. To do this we check
	// the use count of the allocation node and subtract the internal ref count,
	// i.e. if a alloc node has a weight scaler node that increments the shared 
	// pointer use count but is valid. If the use count is greater than 3 then
	// there is an extra copy of the shared pointer somewhere on python side.
	long long use_count = allocation.use_count();
	use_count -= allocation->internalRefCount();
	if (use_count > 3) // pybind11 instance + this
	{
		throw std::runtime_error("Py Allocation node use count expected < 3, found " + std::to_string(use_count));
	}

	return std::make_shared<StrategyNode>(
		std::move(allocation), portfolio
	);
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
Result<SharedPtr<FixedAllocationNode>, AtlasException>
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
SharedPtr<FixedAllocationNode>
FixedAllocationNode::pyMake(Vector<std::pair<String, double>> m_allocations, Exchange* exchange, double epsilon)
{
	auto result = make(std::move(m_allocations), exchange, epsilon);
	if (result)
	{
		return std::move(*result);
	}
	else
	{
		throw std::runtime_error(result.error().what());
	}
	}


//============================================================================
void
FixedAllocationNode::evaluateChild(LinAlg::EigenRef<LinAlg::EigenVectorXd>target) noexcept
{
	for (auto& pair : m_allocations)
	{
		assert(pair.first < static_cast<size_t>(target.size()));
		target(pair.first) = pair.second;
	}
}



}

}