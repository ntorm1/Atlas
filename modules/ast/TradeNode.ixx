module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module TradeNodeModule;

import AtlasCore;
import AtlasLinAlg;

import BaseNodeModule;
import AtlasEnumsModule;

namespace Atlas
{

namespace AST
{

// Define bitwise OR assignment operator for TradeLimitType
TradeLimitType& operator|=(TradeLimitType& lhs, TradeLimitType rhs) {
	return lhs = static_cast<TradeLimitType>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

struct TradeLimitNodeImpl;

//============================================================================
export class TradeLimitNode final 
	: public OpperationNode<void,  LinAlg::EigenRef<LinAlg::EigenVectorXd> , LinAlg::EigenRef<LinAlg::EigenVectorXd>,  LinAlg::EigenRef<LinAlg::EigenVectorXd>>
{
	friend class AllocationBaseNode;

private:
	Exchange const& m_exchange;
	TradeLimitType m_trade_type;
	UniquePtr<TradeLimitNodeImpl> m_impl;
	bool m_is_first_step = true;

	void reset() noexcept;

public:
	ATLAS_API ~TradeLimitNode() noexcept;
	ATLAS_API TradeLimitNode(
		AllocationBaseNode* parent,
		TradeLimitType trade_type,
		double limit
	) noexcept;

	void evaluate(
		 LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl,
		 LinAlg::EigenRef<LinAlg::EigenVectorXd> current_weights,
		 LinAlg::EigenRef<LinAlg::EigenVectorXd> previous_weights
	)noexcept override;

	bool isTradeTypeSet(TradeLimitType type) const noexcept {
		return (m_trade_type & static_cast<unsigned int>(type)) != 0;
	}

	size_t getWarmup() const noexcept override {return 0; }
	void setStopLoss(double stop_loss) noexcept;
	void setTakeProfit(double take_profit) noexcept;
	void setLimit(TradeLimitType trade_type, double limit) noexcept;
};


}

}