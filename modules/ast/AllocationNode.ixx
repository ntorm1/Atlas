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
import PyNodeWrapperModule;

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


struct AllocationBaseNodeImpl;

//============================================================================
export class AllocationBaseNode : public StrategyBufferOpNode
{
protected:
	UniquePtr<AllocationBaseNodeImpl> m_impl;
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
	void setTradeLimit(TradeLimitType t, double limit) noexcept;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept;
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override;
	virtual void evaluateChild(LinAlg::EigenVectorXd& target) noexcept = 0;
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
	ATLAS_API [[nodiscard]] static Result<UniquePtr<AllocationBaseNode>, AtlasException>
		make(
			Vector<std::pair<String, double>> m_allocations,
			Exchange* exchange,
			double epsilon = 0.000f
		) noexcept;


	//============================================================================
	ATLAS_API static PyNodeWrapper<AllocationBaseNode> pyMake(
		Vector<std::pair<String, double>> m_allocations,
		SharedPtr<Exchange> exchange,
		double epsilon = 0.000f
	);

	void evaluateChild(LinAlg::EigenVectorXd& target) noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return 0; }

};


//============================================================================
export class AllocationNode
final
	: public AllocationBaseNode
{
private:
	UniquePtr<ExchangeViewNode> m_exchange_view;

public:
	ATLAS_API ~AllocationNode() noexcept;

	//============================================================================
	ATLAS_API AllocationNode(
		UniquePtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;


	//============================================================================
	ATLAS_API [[nodiscard]] static Result<UniquePtr<AllocationBaseNode>, AtlasException>
		make(
			UniquePtr<ExchangeViewNode> exchange_view,
			AllocationType type = AllocationType::UNIFORM,
			Option<double> alloc_param = std::nullopt,
			double epsilon = 0.000f
		) noexcept;


	//============================================================================
	ATLAS_API static PyNodeWrapper<AllocationBaseNode>
		pyMake(
			PyNodeWrapper<ExchangeViewNode> exchange_view,
			AllocationType type = AllocationType::UNIFORM,
			Option<double> alloc_param = std::nullopt,
			double epsilon = 0.000f
		);


	[[nodiscard]] size_t getWarmup() const noexcept override;
	void evaluateChild(LinAlg::EigenVectorXd& target) noexcept override;
};


}

}