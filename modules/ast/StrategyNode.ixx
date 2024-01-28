module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module StrategyNodeModule;

import AtlasCore;
import AtlasLinAlg;
import BaseNodeModule;
import StrategyBufferModule;
import AllocationNodeModule;
import PyNodeWrapperModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export class StrategyNode final : OpperationNode<bool, LinAlg::EigenVectorXd&>
{
	friend class Strategy;
private:
	UniquePtr<AllocationBaseNode> m_allocation;
	Option<UniquePtr<AllocationWeightNode>> m_alloc_weight;
	Option<SharedPtr<TriggerNode>> m_trigger;
	Portfolio& m_portfolio;
	size_t m_warmup;

	void reset() noexcept;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept;

public:
	//============================================================================
	ATLAS_API ~StrategyNode() noexcept;


	//============================================================================
	ATLAS_API StrategyNode(
		UniquePtr<AllocationBaseNode> allocation,
		Portfolio& portfolio
	) noexcept;


	//============================================================================
	ATLAS_API StrategyNode(
		UniquePtr<AllocationBaseNode> allocation,
		SharedPtr<Portfolio> portfolio
	) noexcept : StrategyNode(std::move(allocation), *portfolio) {}


	//============================================================================
	ATLAS_API [[nodiscard]] static UniquePtr<StrategyNode> make(
		UniquePtr<AllocationBaseNode> allocation,
		Portfolio& portfolio
	) noexcept
	{
		return std::make_unique<StrategyNode>(
			std::move(allocation), portfolio
		);
	}

	//============================================================================
	ATLAS_API [[nodiscard]] static PyNodeWrapper<StrategyNode> pyMake(
		PyNodeWrapper<AllocationNode> allocation,
		Portfolio& portfolio
	);



	ATLAS_API [[nodiscard]] bool evaluate(LinAlg::EigenVectorXd& target) noexcept override;
	ATLAS_API void setTrigger(SharedPtr<TriggerNode> trigger) noexcept;
	ATLAS_API void setWarmupOverride(size_t warmup) noexcept { m_warmup = warmup; }
	
	[[nodiscard]] Portfolio& getPortfolio() const noexcept { return m_portfolio; }
	[[nodiscard]] Exchange& getExchange() noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }

};


}

}