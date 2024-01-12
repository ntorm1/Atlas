module;
#include <Eigen/Dense>
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
	double epsilon
) noexcept:
	OpperationNode<void, Eigen::VectorXd&>(NodeType::ALLOC),
	m_exchange_view(std::move(exchange_view)),
	m_epsilon(epsilon),
	m_exchange(m_exchange_view->getExchange())
{
	m_mask.resize(m_exchange.getAssetCount());
}


//============================================================================
UniquePtr<AllocationNode>
AllocationNode::make(UniquePtr<ExchangeViewNode> exchange_view, double epsilon) noexcept
{
	return std::make_unique<AllocationNode>(
		std::move(exchange_view), epsilon
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

	// generate the boolean mask for assets out of range
	m_mask = (m_exchange.getTradeable().array() > static_cast<int>(getWarmup()));

	// apply the mask to the signal by taking the element-wise product
	assert(target.rows() == m_mask.rows());
	target = target.array() * m_mask.cast<double>();

	// calculate the number of non-NaN elements in the signal
	size_t nonNanCount = (target.array() == target.array()).count();
	size_t zeroCount = (target.array() == 0.0f).count();

	// subtract the out of range assets from the count
	nonNanCount -= zeroCount;
	double c;
	if (nonNanCount > 0) {
		c = (1.0 - m_epsilon) / nonNanCount;
	} else {
		c = 0.0;
	}
	target = target.unaryExpr([c](double x) { return x == x ? c : 0.0; });
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