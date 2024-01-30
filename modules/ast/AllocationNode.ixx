module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module AllocationNodeModule;

import AtlasCore;
import AtlasEnumsModule;
import AtlasLinAlg;
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
	FIXED = 2,
	NLARGEST = 3,
	NSMALLEST = 4,
	NEXTREME = 5
};


struct AllocationBaseNodeImpl;

//============================================================================
export class AllocationBaseNode : public StrategyBufferOpNode
{
	friend class StrategyNode;
protected:
	UniquePtr<AllocationBaseNodeImpl> m_impl;
	
	void reset() noexcept;

public:
	virtual ~AllocationBaseNode() noexcept;
	AllocationBaseNode(
		AllocationType m_type,
		Exchange& exchange,
		double epsilon,
		Option<double> alloc_param
	) noexcept;

	[[nodiscard]] AllocationType getType() const noexcept;
	[[nodiscard]] double getAllocEpsilon() const noexcept;
	[[nodiscard]] virtual size_t getWarmup() const noexcept = 0;
	[[nodiscard]] size_t getAssetCount() const noexcept;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept;
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override;
	virtual void evaluateChild(LinAlg::EigenVectorXd& target) noexcept = 0;

	ATLAS_API void setTradeLimit(TradeLimitType t, double limit) noexcept;
	ATLAS_API Option<TradeLimitNode*> getTradeLimitNode() const noexcept;
};


//============================================================================
export class FixedAllocationNode final
	: public AllocationBaseNode
{
private:
	Vector<std::pair<size_t, double>> m_allocations;

public:
	//============================================================================
	ATLAS_API ~FixedAllocationNode() noexcept;

	//============================================================================
	ATLAS_API FixedAllocationNode(
		Vector<std::pair<size_t, double>> m_allocations,
		Exchange* exchange,
		double epsilon = 0.000f
	) noexcept;


	//============================================================================
	ATLAS_API [[nodiscard]] static Result<SharedPtr<FixedAllocationNode>, AtlasException>
		make(
			Vector<std::pair<String, double>> m_allocations,
			Exchange* exchange,
			double epsilon = 0.000f
		) noexcept;


	void evaluateChild(LinAlg::EigenVectorXd& target) noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return 0; }

};


//============================================================================
export class AllocationNode
final
	: public AllocationBaseNode
{
private:
	Option<size_t> n_alloc_param = std::nullopt;
	SharedPtr<ExchangeViewNode> m_exchange_view;
public:
	ATLAS_API ~AllocationNode() noexcept;

	void setNAllocParam(size_t n) noexcept { n_alloc_param = n; }
	[[nodiscard]] size_t getNAllocParam() const noexcept { return n_alloc_param.value(); }

	//============================================================================
	ATLAS_API AllocationNode(
		SharedPtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;

	//============================================================================
	ATLAS_API [[nodiscard]] static Result<SharedPtr<AllocationNode>, AtlasException>
	make(
			SharedPtr<ExchangeViewNode> exchange_view,
			AllocationType type = AllocationType::UNIFORM,
			Option<double> alloc_param = std::nullopt,
			double epsilon = 0.000f
		) noexcept;


	[[nodiscard]] size_t getWarmup() const noexcept override;
	void evaluateChild(LinAlg::EigenVectorXd& target) noexcept override;
};


}

}