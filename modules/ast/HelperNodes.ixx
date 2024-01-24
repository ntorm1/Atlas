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
	friend class Exchange;
private:
	virtual Result<bool, AtlasException> build() noexcept = 0;

	virtual void step() noexcept = 0;

protected:
	Eigen::VectorXi m_tradeable_mask;	
	size_t m_index_counter = 0;
	Exchange const& m_exchange;

public:
	virtual ~TriggerNode() noexcept = default;
	TriggerNode(Exchange const& exchange) noexcept;
	virtual bool evaluate() noexcept override = 0;
	virtual void reset() noexcept = 0;
	size_t getWarmup() const noexcept override { return 0; }
	Exchange const& getExchange() const noexcept { return m_exchange; }
	Eigen::VectorXi const& getMask() const noexcept { return m_tradeable_mask; }
};


//============================================================================
export class PeriodicTriggerNode : public TriggerNode
{
private:
	size_t m_frequency;
	Result<bool, AtlasException> build() noexcept;
	void step() noexcept;

public:
	ATLAS_API ~PeriodicTriggerNode() noexcept = default;
	ATLAS_API PeriodicTriggerNode(
		Exchange const& exchange,
		size_t frequency
	) noexcept;

	void reset() noexcept override;
	bool evaluate() noexcept override;

	ATLAS_API [[nodiscard]] static SharedPtr<TriggerNode>
	pyMake(
		SharedPtr<Exchange> exchange,
		size_t frequency
	);

};


//============================================================================
export class StrategyMonthlyRunnerNode : public TriggerNode
{
private:
	Result<bool, AtlasException> build() noexcept override;
	void step() noexcept override;
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