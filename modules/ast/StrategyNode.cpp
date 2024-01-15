module;
#include <Eigen/Dense>
#include <iostream>
#include "AtlasMacros.hpp"
module StrategyNodeModule;

import ExchangeModule;

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
AllocationNode::AllocationNode(
	UniquePtr<ExchangeViewNode> exchange_view,
	AllocationType type,
	Option<double> alloc_param,
	double epsilon
) noexcept :
	OpperationNode<void, Eigen::VectorXd&>(NodeType::ALLOC),
	m_exchange_view(std::move(exchange_view)),
	m_epsilon(epsilon),
	m_exchange(m_exchange_view->getExchange()),
	m_type(type),
	m_alloc_param(alloc_param)
{
}


//============================================================================
Result<UniquePtr<AllocationNode>, AtlasException>
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
Exchange&
	AllocationNode::getExchange() noexcept
{
	return m_exchange;
}


//============================================================================
void
	AllocationNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// evaluate the exchange view to calculate the signal
	m_exchange_view->evaluate(target);

	// calculate the number of non-NaN elements in the signal
	size_t nonNanCount = (target.array().isNaN() == false).count();
	size_t zeroCount = (target.array() == 0.0f).count();

	// subtract the out of range assets from the count
	nonNanCount -= zeroCount;
	double c;
	if (nonNanCount > 0) {
		c = (1.0 - m_epsilon) / static_cast<double>(nonNanCount);
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
	UniquePtr<AllocationNode> allocation,
	Portfolio& portfolio
) noexcept :
	OpperationNode<void, Eigen::VectorXd&>(NodeType::STRATEGY),
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
void
	StrategyNode::evaluate(Eigen::VectorXd& target) noexcept
{
	return m_allocation->evaluate(target);
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


}

}