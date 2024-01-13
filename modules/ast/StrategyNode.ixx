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
};


//============================================================================
export class AllocationNode
	final
	: public OpperationNode<void, Eigen::VectorXd&>
{
private:
	UniquePtr<ExchangeViewNode> m_exchange_view;
	Exchange& m_exchange;
	double m_epsilon;
	AllocationType m_type;
	Option<double> m_alloc_param;

public:
	ATLAS_API ~AllocationNode() noexcept;
	ATLAS_API AllocationNode(
		UniquePtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;
	ATLAS_API [[nodiscard]] static Result<UniquePtr<AllocationNode>,AtlasException>
	make(
		UniquePtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;


	[[nodiscard]] double getAllocEpsilon() const noexcept { return m_epsilon; }
	[[nodiscard]] Exchange& getExchange() noexcept;
	[[nodiscard]] void evaluate(Eigen::VectorXd& target) noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override;

};


//============================================================================
export class StrategyNode final : OpperationNode<void, Eigen::VectorXd&>
{
private:
	UniquePtr<AllocationNode> m_allocation;
	Portfolio& m_portfolio;
	size_t m_warmup;
public:
	ATLAS_API StrategyNode(
		UniquePtr<AllocationNode> allocation,
		Portfolio& portfolio
	) noexcept;
	ATLAS_API [[nodiscard]] static UniquePtr<StrategyNode> make(
		UniquePtr<AllocationNode> allocation,
		Portfolio& portfolio
	) noexcept
	{
		return std::make_unique<StrategyNode>(
			std::move(allocation), portfolio
		);
	}
	ATLAS_API ~StrategyNode() noexcept;

	[[nodiscard]] void evaluate(Eigen::VectorXd& target) noexcept override;
	[[nodiscard]] Portfolio& getPortfolio() const noexcept { return m_portfolio; }
	[[nodiscard]] Exchange& getExchange() noexcept;
	[[nodiscard]] double getAllocEpsilon() const noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup;}

};


}

}