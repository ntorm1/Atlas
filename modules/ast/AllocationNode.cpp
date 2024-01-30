module;
#include <Eigen/Dense>

#include "AtlasMacros.hpp"

module AllocationNodeModule;

import ExchangeModule;
import CommissionsModule;
import ExchangeNodeModule;
import TradeNodeModule;


namespace Atlas
{

namespace AST
{


//============================================================================
struct AllocationBaseNodeImpl
{
	AllocationType m_type;
	double m_epsilon;
	Eigen::VectorXd m_weights_buffer;
	Option<double> m_alloc_param = std::nullopt;
	Option<SharedPtr<CommisionManager>> m_commision_manager = std::nullopt;
	Option<UniquePtr<TradeLimitNode>> m_trade_limit = std::nullopt;
};


//============================================================================
void AllocationBaseNode::reset() noexcept
{
	if (m_impl->m_trade_limit)
	{
		(*m_impl->m_trade_limit)->reset();
	}
}

//============================================================================
AllocationBaseNode::~AllocationBaseNode() noexcept
{
}


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
	StrategyBufferOpNode(NodeType::ALLOC, exchange, std::nullopt)
{

	m_impl = std::make_unique<AllocationBaseNodeImpl>();
	m_impl->m_type = m_type;
	m_impl->m_weights_buffer.resize(exchange.getAssetCount());
	m_impl->m_weights_buffer.setZero();
	m_impl->m_epsilon = epsilon;
	m_impl->m_alloc_param = alloc_param;
}


//============================================================================
AllocationType AllocationBaseNode::getType() const noexcept
{
	return m_impl->m_type;

}


//============================================================================
double
AllocationBaseNode::getAllocEpsilon() const noexcept
{
	return m_impl->m_epsilon;
}


//============================================================================
size_t
AllocationBaseNode::getAssetCount() const noexcept
{
	return m_exchange.getAssetCount();
}

//============================================================================
void
AllocationBaseNode::setTradeLimit(TradeLimitType t, double limit) noexcept
{
	m_impl->m_trade_limit = std::make_unique<TradeLimitNode>(this, t, limit);
}


//============================================================================
void
AllocationBaseNode::setCommissionManager(SharedPtr<CommisionManager> manager) noexcept
{
	m_impl->m_commision_manager = manager;
}


//============================================================================
Option<TradeLimitNode*>
AllocationBaseNode::getTradeLimitNode() const noexcept
{
	if (m_impl->m_trade_limit)
	{
		return m_impl->m_trade_limit->get();
	}
	else
	{
		return std::nullopt;
	}
}


//============================================================================
void
AllocationBaseNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// if we have a commission manager or trade watcher we need to copy the current weights buffer 
	// into the commission manager buffer before it gets overwritten by the ast. 
	// 
	// Additionally if weight epsilon is set we need to copy the current weights
	// so that any adjustments made are reverted back if the absolute value of the
	// differene is less than epsilon
	if (
		m_impl->m_commision_manager||
		m_impl->m_epsilon ||
		m_impl->m_trade_limit
		)
	{
		m_impl->m_weights_buffer = target;
	}

	evaluateChild(target);

	// if epsilon is set we need to check if the difference between the current
	// weights and the new weights is less than epsilon. If it is we need to
	// revert the weights back to the original weights before calculating any commissions
	if (m_impl->m_epsilon)
	{
		target = ((target - m_impl->m_weights_buffer).cwiseAbs().array() < m_impl->m_epsilon)
			.select(m_impl->m_weights_buffer, target);
	}

	// if we have stop loss or take profit we need to check if the trade limits have been exceeded
	// by passing in the previous weights to compute the pnl, then adjust target weights accordingly
	if (m_impl->m_trade_limit)
	{
		(*m_impl->m_trade_limit)->evaluate(target, m_impl->m_weights_buffer);
	}

	// if we have a commission manager we need to calculate the commission caused
	// by the current weights adjustment
	if (m_impl->m_commision_manager && (*m_impl->m_commision_manager)->hasCommission())
	{
		(*m_impl->m_commision_manager)->calculateCommission(target, m_impl->m_weights_buffer);
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

	auto node = std::make_unique<AllocationNode>(
		std::move(exchange_view), type, alloc_param, epsilon
	);

	if (type == AllocationType::NEXTREME)
	{
		if (!alloc_param)
		{
			return Err("Allocation type NEXTREME requires a parameter");
		}
		// attempt to cast the alloc_param to a size_t to use as the number of assets to allocate
		// ussing n extreme values
		try
		{
			node->setNAllocParam(static_cast<size_t>(*alloc_param));
		}
		catch (std::exception&)
		{
			return Err("Allocation type NEXTREME requires a size_t parameter");
		}
		// n*2 must be less than the number of assets in the exchange
		size_t asset_count = exchange_view->getExchange().getAssetCount();
		if (node->getNAllocParam() * 2 >= asset_count)
		{
			return Err("Allocation type NEXTREME requires a parameter less than half the number of assets in the exchange");
		}
	}
	return std::move(node);
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

	switch (m_impl->m_type) {
	case AllocationType::NLARGEST:
	case AllocationType::NSMALLEST:
	case AllocationType::UNIFORM: {
		target = target.unaryExpr([c](double x) { return x == x ? c : 0.0; });
		break;
	}
	case AllocationType::CONDITIONAL_SPLIT: {
		target = (target.array() < *m_impl->m_alloc_param)
			.select(-c, (target.array() > *m_impl->m_alloc_param)
				.select(c, target));
		target = target.unaryExpr([](double x) { return x == x ? x : 0.0; });
		break;
	}
	case AllocationType::NEXTREME:
		assert(false); // not implemented
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