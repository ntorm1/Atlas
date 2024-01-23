module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module StrategyNodeModule;

import AtlasCore;
import BaseNodeModule;

namespace Atlas
{

namespace AST
{

export enum class AllocationType
{
	UNIFORM = 0,
	CONDITIONAL_SPLIT = 1,
	FIXED = 2
};


export class AllocationBaseNode : public OpperationNode<void, Eigen::VectorXd&>
{
protected:
	AllocationType m_type;
	Exchange& m_exchange;
	double m_epsilon;
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
	[[nodiscard]] Exchange& getExchange() noexcept;
	void setCommisionManager(SharedPtr<CommisionManager> manager) noexcept { m_commision_manager = manager; }
	void evaluateBase(Eigen::VectorXd& target) noexcept;
	void evaluate(Eigen::VectorXd& target) noexcept override = 0;
};


//============================================================================
export class FixedAllocationNode final 
	: public AllocationBaseNode
{
private:
	Vector<std::pair<size_t, double>> m_allocations;

public:
	ATLAS_API ~FixedAllocationNode() noexcept;
	ATLAS_API FixedAllocationNode(
		Vector<std::pair<size_t, double>> m_allocations,
		Exchange* exchange,
		double epsilon = 0.000f
	) noexcept;

	ATLAS_API [[nodiscard]] static Result<SharedPtr<AllocationBaseNode>, AtlasException>
	make(
			Vector<std::pair<String, double>> m_allocations,
			Exchange* exchange,
			double epsilon = 0.000f
	) noexcept;

	ATLAS_API static SharedPtr<AllocationBaseNode> pyMake(
		Vector<std::pair<String, double>> m_allocations,
		SharedPtr<Exchange> exchange,
		double epsilon = 0.000f
	);

	void evaluate(Eigen::VectorXd& target) noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return 0; }

};



//============================================================================
export class AllocationNode
	final
	: public AllocationBaseNode
{
private:
	SharedPtr<ExchangeViewNode> m_exchange_view;

public:
	ATLAS_API ~AllocationNode() noexcept;
	ATLAS_API AllocationNode(
		SharedPtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;

	ATLAS_API [[nodiscard]] static Result<SharedPtr<AllocationBaseNode>, AtlasException>
	make(
		SharedPtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;


	[[nodiscard]] size_t getWarmup() const noexcept override;
	void evaluate(Eigen::VectorXd& target) noexcept override;
};


//============================================================================
export class StrategyNode final : OpperationNode<bool, Eigen::VectorXd&>
{
	friend class Strategy;
private:
	SharedPtr<AllocationBaseNode> m_allocation;
	Option<SharedPtr<TriggerNode>> m_trigger;
	Option<SharedPtr<AllocationWeightNode>> m_alloc_weight;
	Portfolio& m_portfolio;
	size_t m_warmup;

	void reset() noexcept;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept { m_allocation->setCommisionManager(manager); }

public:
	ATLAS_API StrategyNode(
		SharedPtr<AllocationBaseNode> allocation,
		Portfolio& portfolio
	) noexcept;

	ATLAS_API StrategyNode(
		SharedPtr<AllocationBaseNode> allocation,
		SharedPtr<Portfolio> portfolio
	) noexcept : StrategyNode(std::move(allocation), *portfolio) {}

	ATLAS_API [[nodiscard]] static SharedPtr<StrategyNode> make(
		SharedPtr<AllocationBaseNode> allocation,
		Portfolio& portfolio
	) noexcept
	{
		return std::make_unique<StrategyNode>(
			std::move(allocation), portfolio
		);
	}
	ATLAS_API ~StrategyNode() noexcept;

	ATLAS_API [[nodiscard]] bool evaluate(Eigen::VectorXd& target) noexcept override;
	ATLAS_API void setTrigger(SharedPtr<TriggerNode> trigger) noexcept;

	[[nodiscard]] Portfolio& getPortfolio() const noexcept { return m_portfolio; }
	[[nodiscard]] Exchange& getExchange() noexcept;
	[[nodiscard]] double getAllocEpsilon() const noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }

};


}

}