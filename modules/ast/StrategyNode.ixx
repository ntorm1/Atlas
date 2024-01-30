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
export class StrategyNode final : OpperationNode<bool, LinAlg::EigenVectorXd&>
{
	friend class Strategy;
private:
	SharedPtr<AllocationBaseNode> m_allocation;
	Option<SharedPtr<AllocationWeightNode>> m_alloc_weight;
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
		SharedPtr<AllocationBaseNode> allocation,
		Portfolio& portfolio
	) noexcept;


	//============================================================================
	ATLAS_API StrategyNode(
		SharedPtr<AllocationBaseNode> allocation,
		SharedPtr<Portfolio> portfolio
	) noexcept : StrategyNode(std::move(allocation), *portfolio) {}


	//============================================================================
	ATLAS_API [[nodiscard]] static SharedPtr<StrategyNode> make(
		SharedPtr<AllocationBaseNode> allocation,
		Portfolio& portfolio
	)
	{
		if (allocation.use_count() > 3) // pybind11 instance + this
		{
			auto use_count = allocation.use_count();
			throw std::runtime_error("Allocation node use count expected < 2, found " + std::to_string(use_count));
		}

		return std::make_shared<StrategyNode>(
			std::move(allocation), portfolio
		);
	}


	ATLAS_API [[nodiscard]] bool evaluate(LinAlg::EigenVectorXd& target) noexcept override;
	ATLAS_API void setTrigger(SharedPtr<TriggerNode> trigger) noexcept;
	ATLAS_API void setWarmupOverride(size_t warmup) noexcept { m_warmup = warmup; }
	
	[[nodiscard]] Portfolio& getPortfolio() const noexcept { return m_portfolio; }
	[[nodiscard]] Exchange& getExchange() noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }

};


}

}