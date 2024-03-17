module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <stdexcept>
#include <string>
export module StrategyNodeModule;

import AtlasCore;
import AtlasLinAlg;
import BaseNodeModule;
import StrategyBufferModule;
import AllocationNodeModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export class StrategyNode final : OpperationNode<bool, LinAlg::EigenRef<LinAlg::EigenVectorXd>>
{
	friend class Strategy;
private:
	SharedPtr<AllocationBaseNode> m_allocation;
	Option<SharedPtr<AllocationWeightNode>> m_alloc_weight;
	Option<SharedPtr<TriggerNode>> m_trigger;
	size_t m_warmup;

	[[nodiscard]] size_t refreshWarmup() noexcept;
	void reset() noexcept;
	void setTracer(SharedPtr<Tracer> tracer) noexcept;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept;
	void enableCopyWeightsBuffer() noexcept;
	Option<SharedPtr<TradeLimitNode>> getTradeLimitNode() const noexcept;
	LinAlg::EigenRef<LinAlg::EigenVectorXd> getPnL() noexcept;


public:
	//============================================================================
	ATLAS_API ~StrategyNode() noexcept;


	//============================================================================
	ATLAS_API StrategyNode(
		SharedPtr<AllocationBaseNode> allocation
	) noexcept;


	//============================================================================
	ATLAS_API [[nodiscard]] static SharedPtr<StrategyNode> make(
		SharedPtr<AllocationBaseNode> allocation
	);


	ATLAS_API [[nodiscard]] bool evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	ATLAS_API void setTrigger(SharedPtr<TriggerNode> trigger) noexcept;
	ATLAS_API void setWarmupOverride(size_t warmup) noexcept { m_warmup = warmup; }
	
	[[nodiscard]] Exchange& getExchange() noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }

};


}

}