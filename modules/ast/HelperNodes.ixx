module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module HelperNodesModule;

import BaseNodeModule;
import AtlasCore;

namespace Atlas
{

namespace AST
{

//============================================================================
export class TriggerNode 
	: public ExpressionNode<bool>
{
private:
	virtual Result<bool, AtlasException> build() noexcept = 0;

protected:
	Eigen::VectorXi m_tradeable_mask;	
	size_t m_index_counter = 0;
	Exchange const& m_exchange;
public:
	
	TriggerNode(Exchange const& exchange
	)  noexcept : 
		m_exchange(exchange),
		ExpressionNode<bool>(NodeType::STRATEGY_RUNNER)
	{}

	virtual bool evaluate() noexcept override = 0;
	size_t getWarmup() const noexcept override { return 0; }
};


//============================================================================
export class StrategyMonthlyRunnerNode : public TriggerNode
{
private:
	virtual Result<bool, AtlasException> build() noexcept override;

public:
	StrategyMonthlyRunnerNode(
		Strategy const& strategy
	) noexcept;

	bool evaluate() noexcept override;

	ATLAS_API [[nodiscard]] static Result<UniquePtr<TriggerNode>,AtlasException>
	make(
		Strategy const& strategy
	) noexcept;

};




}

}