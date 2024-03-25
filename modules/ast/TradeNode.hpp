#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include "standard/AtlasCore.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "standard/AtlasEnums.hpp"
#include "ast/BaseNode.hpp"

namespace Atlas
{

namespace AST
{


struct TradeLimitNodeImpl;

//============================================================================
class TradeLimitNode final 
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

	void unsetTradeType(TradeLimitType type) noexcept {
		m_trade_type &= ~static_cast<unsigned int>(type);
	}

	size_t getWarmup() const noexcept override {return 0; }
	void setLimit(TradeLimitType trade_type, double limit) noexcept;
	ATLAS_API static double getStopLoss(SharedPtr<TradeLimitNode> node) noexcept;
	ATLAS_API static double getTakeProfit(SharedPtr<TradeLimitNode> node) noexcept;
	ATLAS_API static void setStopLoss(SharedPtr<TradeLimitNode> node, double stopLoss) noexcept;
	ATLAS_API static void setTakeProfit(SharedPtr<TradeLimitNode> node, double takeProfit) noexcept;
	
	ATLAS_API uintptr_t getStopLossGetter() const noexcept {
		return reinterpret_cast<uintptr_t>(&TradeLimitNode::getStopLoss);
	}
	ATLAS_API uintptr_t getTakeProfitGetter() const noexcept {
		return reinterpret_cast<uintptr_t>(&TradeLimitNode::getTakeProfit);
	}
	ATLAS_API uintptr_t getStopLossSetter() const noexcept {
		return reinterpret_cast<uintptr_t>(&TradeLimitNode::setStopLoss);
	}
	ATLAS_API uintptr_t getTakeProfitSetter() const noexcept {
		return reinterpret_cast<uintptr_t>(&TradeLimitNode::setTakeProfit);
	}
};


}

}