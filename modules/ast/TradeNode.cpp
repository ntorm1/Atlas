module;
#include <Eigen/Dense>
module TradeNodeModule;

import AllocationNodeModule;
import ExchangeModule;

namespace Atlas
{

namespace AST
{


//============================================================================
struct TradeLimitNodeImpl
{
	Eigen::VectorXd m_pnl;
	TradeLimitType m_trade_type;
	double m_limit;

	TradeLimitNodeImpl(TradeLimitType trade_type, double limit) noexcept:
		m_trade_type(trade_type),
		m_limit(limit)
	{
	}
};

//============================================================================
TradeLimitNode::~TradeLimitNode() noexcept
{
}


//============================================================================
TradeLimitNode::TradeLimitNode(
	AllocationBaseNode* parent,
	TradeLimitType trade_type,
	double limit
) noexcept:
	OpperationNode<void, LinAlg::EigenVectorXd&, LinAlg::EigenVectorXd&>(NodeType::TRADE_LIMIT, parent),
	m_exchange(parent->getExchange())
{
	switch (trade_type)
	{
		case TradeLimitType::STOP_LOSS:
			// stop loss is (1 - limit) i.e. .05 -> .95
			limit = (1 - limit);
		break;
		case TradeLimitType::TAKE_PROFIT:
			// take profit is (1 + limit) i.e. .05 -> 1.05
			limit = (1 + limit);
			break;
	}

	m_impl = std::make_unique<TradeLimitNodeImpl>(trade_type, limit);
	size_t asset_count = parent->getAssetCount();
	m_impl->m_pnl.resize(asset_count);
	m_impl->m_pnl.setZero();
}


//============================================================================
void TradeLimitNode::evaluate(
	LinAlg::EigenVectorXd& current_weights,
	LinAlg::EigenVectorXd& previous_weights
) noexcept
{
	// in the first step we can't have pnl as allocation node is evaluated after the 
	// portfolio is priced. And it would cause index error trying to get the previous steps
	// returns as we see below.
	if (m_is_first_step)
	{
		return;
	}

	// init the pnl trade vector to 1 where the trade pct switched sign or 
	// went from 0 to non-zero
	m_impl->m_pnl = m_impl->m_pnl.select(
		(current_weights.array() != 0) && ((previous_weights.array() == 0)
		||
		(current_weights.array() * previous_weights.array() < 0)), 1.0f).cast<double>();

	// Trade limit node evaluate is called before the allocation node updates.
	// Therefore, current_weights hold the weights that were used for the most 
	// recent evaluation. So we need to update the trade pnl vector to adjust the pnl
	// Stat by pulling in the previous market returns using index offset.
	Eigen::VectorXd previous_returns = m_exchange.getMarketReturns(-1);
	previous_returns.array() += 1.0;

	// multiply 1 + returns to get the new price using the m_pnl buffer
	m_impl->m_pnl = m_impl->m_pnl.cwiseProduct(previous_returns);

	// switch on the trade type to zero out weights as required. For stop loss
	// we zero out the weights when the pnl is less than the limit. For take profit
	// we zero out the weights when the pnl is greater than the limit.
	switch (m_impl->m_trade_type)
	{
	case TradeLimitType::STOP_LOSS:
		current_weights = current_weights.array().cwiseProduct(
			(m_impl->m_pnl.array() < m_impl->m_limit).cast<double>()
		);		
		break;
	case TradeLimitType::TAKE_PROFIT:
		current_weights = current_weights.array().cwiseProduct(
			(m_impl->m_pnl.array() > m_impl->m_limit).cast<double>()
		);	
		break;
	}
}

}

}