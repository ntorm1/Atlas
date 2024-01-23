module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module AllocationNodeModule;

import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export enum class AllocationType
{
	UNIFORM = 0,
	CONDITIONAL_SPLIT = 1,
	FIXED = 2
};


//============================================================================
export class AllocationBaseNode : public StrategyBufferOpNode
{
protected:
	AllocationType m_type;
	double m_epsilon;
	Eigen::VectorXd m_weights_buffer;
	Option<double> m_alloc_param = std::nullopt;
	Option<SharedPtr<CommisionManager>> m_commision_manager = std::nullopt;
public:
	virtual ~AllocationBaseNode() noexcept = default;
	AllocationBaseNode(
		AllocationType m_type,
		Exchange& exchange,
		double epsilon,
		Option<double> alloc_param
	) noexcept;

	[[nodiscard]] AllocationType getType() const noexcept { return m_type; }
	[[nodiscard]] double getAllocEpsilon() const noexcept { return m_epsilon; }
	[[nodiscard]] virtual size_t getWarmup() const noexcept = 0;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept { m_commision_manager = manager; }
	void evaluate(Eigen::VectorXd& target) noexcept override;
	virtual void evaluateChild(Eigen::VectorXd& target) noexcept = 0;
};

}

}