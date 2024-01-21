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

static Int64 applyDateOffset();

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
	virtual ~TriggerNode() noexcept = default;
	TriggerNode(Exchange const& exchange
	)  noexcept : 
		m_exchange(exchange),
		ExpressionNode<bool>(NodeType::STRATEGY_RUNNER)
	{}

	virtual bool evaluate() noexcept override = 0;
	virtual void reset() noexcept = 0;
	size_t getWarmup() const noexcept override { return 0; }
	Exchange const& getExchange() const noexcept { return m_exchange; }
	Eigen::VectorXi const& getMask() const noexcept { return m_tradeable_mask; }
};


//============================================================================
export class StrategyMonthlyRunnerNode : public TriggerNode
{
private:
	virtual Result<bool, AtlasException> build() noexcept override;
	bool m_eom_trigger;
public:
	ATLAS_API ~StrategyMonthlyRunnerNode() noexcept = default;
	ATLAS_API StrategyMonthlyRunnerNode(
		Exchange const& exchange,
		bool eom_trigger = false
	) noexcept;

	void reset() noexcept override;
	bool evaluate() noexcept override;

	ATLAS_API [[nodiscard]] static SharedPtr<TriggerNode>
	pyMake(
		SharedPtr<Exchange> exchange,
		bool eom_trigger = false
	);

};


}

}