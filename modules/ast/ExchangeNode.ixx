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
		SharedPtr<AssetReadNode>,
		SharedPtr<AssetProductNode>,
		SharedPtr<AssetQuotientNode>,
		SharedPtr<AssetSumNode>,
		SharedPtr<AssetDifferenceNode>
	> value;
	size_t warmup;
	AssetNodeType  t;

public:
	using node_variant = std::variant <
		SharedPtr<AssetReadNode>,
		SharedPtr<AssetProductNode>,
		SharedPtr<AssetQuotientNode>,
		SharedPtr<AssetSumNode>,
		SharedPtr<AssetDifferenceNode>>;
	AssetOpNodeVariant() = delete;
	ATLAS_API ~AssetOpNodeVariant() noexcept;
	AssetOpNodeVariant(const AssetOpNodeVariant&) = default;
	AssetOpNodeVariant(AssetOpNodeVariant&&) = default;
	AssetOpNodeVariant& operator=(const AssetOpNodeVariant&) = default;
	AssetOpNodeVariant& operator=(AssetOpNodeVariant&&) = default;
	ATLAS_API AssetOpNodeVariant(node_variant node) noexcept;
};


//============================================================================
export enum class ExchangeViewFilterType
{
	GREATER_THAN = 0,
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
	Option<ExchangeViewFilter> m_filter = std::nullopt;
public:

	ATLAS_API ~ExchangeViewNode() noexcept;
	ATLAS_API ExchangeViewNode(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept;
	ATLAS_API ExchangeViewNode(
		SharedPtr<Exchange> exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept:
		ExchangeViewNode(*exchange, std::move(asset_op_node)) {}

	ATLAS_API [[nodiscard]] static UniquePtr<ExchangeViewNode> make(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept
	{
		return std::make_unique<ExchangeViewNode>(
			exchange, std::move(asset_op_node)
		);

	}
	ATLAS_API void setFilter(ExchangeViewFilterType type, double value) noexcept
	{
		m_filter = ExchangeViewFilter(type, value);
	}

	[[nodiscard]] AssetNodeType getType() const noexcept { return m_asset_op_node.t; }
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] auto& getExchange() { return m_exchange; }
	void filter(Eigen::VectorXd& v) const noexcept;

	ATLAS_API  void evaluate(Eigen::VectorXd&) noexcept override;
};


};

}