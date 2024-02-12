module;
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

#include <Eigen/Dense>
module TradeNodeModule;

import AllocationNodeModule;
import AtlasEnumsModule;
import ExchangeModule;

namespace Atlas
{

namespace AST
{

//============================================================================
struct TradeLimitNodeImpl
{
	double m_stop_loss = 0.0;
	double m_take_profit = 0.0;

	TradeLimitNodeImpl(TradeLimitType trade_type, double limit) noexcept
	{
		switch (trade_type)
		{
			case TradeLimitType::STOP_LOSS:
				m_stop_loss = limit;
				break;
			case TradeLimitType::TAKE_PROFIT:
				m_take_profit = limit;
				break;
		}
	}
};


//============================================================================
void
TradeLimitNode::reset() noexcept
{
	m_is_first_step = true;
}


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
	OpperationNode<void,  LinAlg::EigenRef<LinAlg::EigenVectorXd>, LinAlg::EigenRef<LinAlg::EigenVectorXd>, LinAlg::EigenRef<LinAlg::EigenVectorXd>>
		(NodeType::TRADE_LIMIT, parent),
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
	m_trade_type |= trade_type;
	size_t asset_count = parent->getAssetCount();
}


//============================================================================
void TradeLimitNode::evaluate(
	 LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl,
	 LinAlg::EigenRef<LinAlg::EigenVectorXd> current_weights,
	 LinAlg::EigenRef<LinAlg::EigenVectorXd> previous_weights
) noexcept
{
	// in the first step we can't have pnl as allocation node is evaluated after the 
	// portfolio is priced. And it would cause index error trying to get the previous steps
	// returns as we see below.

	if (m_is_first_step)
	{
		m_is_first_step = false;
	}
	else
	{
		// Trade limit node evaluate is called before the allocation node updates.
		// Therefore, current_weights hold the weights that were used for the most 
		// recent evaluation. So we need to update the trade pnl vector to adjust the pnl
		// Stat by pulling in the previous market returns using index offset and making a mutable copy of the view
		Eigen::VectorXd previous_returns = m_exchange.getMarketReturns();
		previous_returns.array() += 1.0;

		// multiply 1 + returns to get the new price using the m_pnl buffer
		pnl = pnl.cwiseProduct(previous_returns);

		// switch on the trade type to zero out weights as required. For stop loss
		// we zero out the weights when the pnl is less than the limit. For take profit
		// we zero out the weights when the pnl is greater than the limit. Add an additional
		// comparison to 0.0 to prevent the new weights from being zeroed out beforehand
		if (isTradeTypeSet(TradeLimitType::STOP_LOSS))
		{
			current_weights = current_weights.array().cwiseProduct(
				((pnl.array() > m_impl->m_stop_loss) || (pnl.array() == 0.0)).cast<double>()
			);
		}
		if (isTradeTypeSet(TradeLimitType::TAKE_PROFIT))
		{
		current_weights = current_weights.array().cwiseProduct(
				((pnl.array() < m_impl->m_take_profit) || (pnl.array() == 0.0)).cast<double>()
			);
		}
	}

	// init the pnl trade vector to 1 where the trade pct switched sign or 
	// went from 0 to non-zero
	pnl = (
		(current_weights.array() * previous_weights.array() < 0.0f)
		||
		(previous_weights.array() == 0) && (current_weights.array() != 0)
		)
		.select(1.0f, pnl);

	// update closed trade. If the current weight is 0 and the previous weight is not 0
	// then zero out the pnl vector
	pnl = (
		(current_weights.array() == 0)
		&& 
		(previous_weights.array() != 0)
		)
		.select(0.0f, pnl);
}

	
//============================================================================
double
TradeLimitNode::getStopLoss(SharedPtr<TradeLimitNode> node) noexcept
{
	if (node->isTradeTypeSet(TradeLimitType::STOP_LOSS))
	{
		return node->m_impl->m_stop_loss;
	}
	return 0.0;
}

//============================================================================
double
TradeLimitNode::getTakeProfit(SharedPtr<TradeLimitNode> node) noexcept
{
	if (node->isTradeTypeSet(TradeLimitType::TAKE_PROFIT))
	{
		return node->m_impl->m_take_profit;
	}
	return 0.0;
}

//============================================================================
void
TradeLimitNode::setStopLoss(SharedPtr<TradeLimitNode> node, double stopLoss) noexcept
{
	if (stopLoss == 0.0)
	{
		node->unsetTradeType(TradeLimitType::STOP_LOSS);
		return;
	}
	node->m_impl->m_stop_loss = 1 - stopLoss;
	node->m_trade_type |= TradeLimitType::STOP_LOSS;
}

//============================================================================
void
TradeLimitNode::setTakeProfit(SharedPtr<TradeLimitNode> node, double takeProfit) noexcept
{
	if (takeProfit == 0.0)
	{
		node->unsetTradeType(TradeLimitType::TAKE_PROFIT);
		return;
	}
	node->m_impl->m_take_profit = 1 + takeProfit;
	node->m_trade_type |= TradeLimitType::TAKE_PROFIT;
}

//============================================================================
void TradeLimitNode::setLimit(TradeLimitType trade_type, double limit) noexcept
{
	switch (trade_type)
	{
		case TradeLimitType::STOP_LOSS:
			if (limit == 0.0)
			{
				unsetTradeType(TradeLimitType::STOP_LOSS);
				return;
			}
			m_impl->m_stop_loss = 1 - limit;
			m_trade_type |= TradeLimitType::STOP_LOSS;
			break;
		case TradeLimitType::TAKE_PROFIT:
			if (limit == 0.0)
			{
				unsetTradeType(TradeLimitType::TAKE_PROFIT);
				return;
			}
			m_impl->m_take_profit = 1 + limit;
			m_trade_type |= TradeLimitType::TAKE_PROFIT;
			break;
	}
}

}

}