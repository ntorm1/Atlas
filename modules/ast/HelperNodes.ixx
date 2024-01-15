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
export class StrategyRunnerNode 
	: public ExpressionNode<bool>
{
private:
	virtual Result<bool, AtlasException> build() noexcept = 0;

protected:
	Eigen::VectorXi m_tradeable_mask;	
	size_t m_index_counter = 0;
	Exchange const& m_exchange;
public:
	
	StrategyRunnerNode(Exchange const& exchange
	)  noexcept : 
		m_exchange(exchange),
		ExpressionNode<bool>(NodeType::STRATEGY_RUNNER)
	{}
	virtual bool evaluate() noexcept override = 0;
	size_t getWarmup() const noexcept override { return 0; }

};


//============================================================================
export class StrategyMonthlyRunnerNode : public StrategyRunnerNode
{
private:
	virtual Result<bool, AtlasException> build() noexcept override;

public:
	StrategyMonthlyRunnerNode(
		Exchange const& exchange
	) noexcept;

	bool evaluate() noexcept override;

	ATLAS_API [[nodiscard]] static Result<UniquePtr<StrategyRunnerNode>,AtlasException>
	make(
		Exchange const& exchange
	) noexcept;

};




}

}