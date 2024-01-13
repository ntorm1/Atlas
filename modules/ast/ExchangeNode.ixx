module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module ExchangeNodeModule;

import <variant>;

import AtlasCore;
import BaseNodeModule;
import AtlasLinAlg;
import AssetNodeModule;


namespace Atlas
{

namespace AST
{

export enum class AssetNodeType {
	AssetReadNode,
	AssetProductNode,
	AssetQuotientNode,
	AssetSumNode,
	AssetDifferenceNode
};


//============================================================================
// Define AssetOpNodeVariant using std::variant
export class AssetOpNodeVariant {
public:
	std::variant<
		std::unique_ptr<AssetReadNode>,
		std::unique_ptr<AssetProductNode>,
		std::unique_ptr<AssetQuotientNode>,
		std::unique_ptr<AssetSumNode>,
		std::unique_ptr<AssetDifferenceNode>
	> value;
	size_t warmup;
	AssetNodeType  t;

	public:
		using node_variant = std::variant <
			std::unique_ptr<AssetReadNode>,
			std::unique_ptr<AssetProductNode>,
			std::unique_ptr<AssetQuotientNode>,
			std::unique_ptr<AssetSumNode>,
			std::unique_ptr<AssetDifferenceNode>>;
	AssetOpNodeVariant() = delete;
	ATLAS_API ~AssetOpNodeVariant() noexcept;
	AssetOpNodeVariant(const AssetOpNodeVariant&) = delete;
	AssetOpNodeVariant(AssetOpNodeVariant&&) = default;
	AssetOpNodeVariant& operator=(const AssetOpNodeVariant&) = delete;
	AssetOpNodeVariant& operator=(AssetOpNodeVariant&&) = default;
	ATLAS_API AssetOpNodeVariant(node_variant node) noexcept;
};


//============================================================================
export enum class ExchangeViewFilterType
{
	GREATER_THAN=0,
};


//============================================================================
export struct ExchangeViewFilter
{
	ExchangeViewFilterType type;
	double value;
	
	ExchangeViewFilter() = delete;
	ATLAS_API ~ExchangeViewFilter() noexcept = default;
	ATLAS_API ExchangeViewFilter(
		ExchangeViewFilterType type,
		double value
	)  noexcept :
		type(type), value(value) {}
};


//============================================================================
export class ExchangeViewNode
	final
	: public OpperationNode<void, Eigen::VectorXd&>
{
private:
	Exchange& m_exchange;
	AssetOpNodeVariant m_asset_op_node;
	size_t m_warmup;
	Option<ExchangeViewFilter> m_filter;
public:

	ATLAS_API ~ExchangeViewNode() noexcept;
	ATLAS_API ExchangeViewNode(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node,
		Option<ExchangeViewFilter> m_filter
	) noexcept;

	ATLAS_API  [[nodiscard]] static UniquePtr<ExchangeViewNode> make(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node,
		Option<ExchangeViewFilter> m_filter = std::nullopt
	) noexcept
	{
		return std::make_unique<ExchangeViewNode>(
			exchange, std::move(asset_op_node), std::move(m_filter)
		);
	
	}

	[[nodiscard]] AssetNodeType getType() const noexcept { return m_asset_op_node.t; }
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] auto& getExchange() { return m_exchange; }
	void filter(Eigen::VectorXd& v) const noexcept;

	ATLAS_API  void evaluate(Eigen::VectorXd&) noexcept override;
};


};

}