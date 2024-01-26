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


struct TradeLimitNodeImpl;

//============================================================================
export class TradeLimitNode final 
	: public OpperationNode<void, LinAlg::EigenVectorXd&,LinAlg::EigenVectorXd&>
{
	friend class AllocationBaseNode;

private:
	Exchange const& m_exchange;
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
		LinAlg::EigenVectorXd& current_weights,
		LinAlg::EigenVectorXd& previous_weights
	)noexcept override;
	size_t getWarmup() const noexcept override {return 0; }

	ATLAS_API LinAlg::EigenVectorXd const& getPnl() const noexcept;
};


}

}