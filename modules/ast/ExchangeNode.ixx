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
import StrategyBufferModule;
import PyNodeWrapperModule;

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
		UniquePtr<AssetReadNode>,
		UniquePtr<AssetProductNode>,
		UniquePtr<AssetQuotientNode>,
		UniquePtr<AssetSumNode>,
		UniquePtr<AssetDifferenceNode>
	> value;
	size_t warmup;
	AssetNodeType  t;

public:
	using node_variant = std::variant <
		UniquePtr<AssetReadNode>,
		UniquePtr<AssetProductNode>,
		UniquePtr<AssetQuotientNode>,
		UniquePtr<AssetSumNode>,
		UniquePtr<AssetDifferenceNode>>;

	using py_node_variant = std::variant <
		PyNodeWrapper<AssetReadNode>,
		PyNodeWrapper<AssetProductNode>,
		PyNodeWrapper<AssetQuotientNode>,
		PyNodeWrapper<AssetSumNode>,
		PyNodeWrapper<AssetDifferenceNode>>;

	AssetOpNodeVariant(
		node_variant node,
		size_t warmup,
		AssetNodeType t
	) noexcept:
		value(std::move(node)), warmup(warmup), t(t) {}
	AssetOpNodeVariant() = delete;
	AssetOpNodeVariant(const AssetOpNodeVariant&) = default;
	AssetOpNodeVariant(AssetOpNodeVariant&&) = default;
	AssetOpNodeVariant& operator=(const AssetOpNodeVariant&) = default;
	AssetOpNodeVariant& operator=(AssetOpNodeVariant&&) = default;
	ATLAS_API AssetOpNodeVariant(node_variant node) noexcept;
	ATLAS_API static AssetOpNodeVariant pyMake(py_node_variant node) noexcept;
	ATLAS_API ~AssetOpNodeVariant() noexcept;
};


//============================================================================
export enum class ExchangeViewFilterType
{
	GREATER_THAN = 0,
	LESS_THAN = 1,
	EQUAL_TO = 2
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
	: public StrategyBufferOpNode
{
private:
	Exchange& m_exchange;
	AssetOpNodeVariant m_asset_op_node;
	size_t m_view_size;
	size_t m_warmup;
	Option<ExchangeViewFilter> m_filter = std::nullopt;
public:
	ATLAS_API ~ExchangeViewNode() noexcept;

	//============================================================================
	ATLAS_API ExchangeViewNode(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept;


	//============================================================================
	ATLAS_API ExchangeViewNode(
		SharedPtr<Exchange> exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept;


	//============================================================================
	ATLAS_API [[nodiscard]] static UniquePtr<ExchangeViewNode> make(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept
	{
		return std::make_unique<ExchangeViewNode>(
			exchange, std::move(asset_op_node)
		);
	}


	//============================================================================
	ATLAS_API [[nodiscard]] static PyNodeWrapper<ExchangeViewNode> pyMake(
		Exchange& exchange,
		AssetOpNodeVariant asset_op_node
	) noexcept
	{
		auto node = std::make_unique<ExchangeViewNode>(
			exchange, std::move(asset_op_node)
		);
		return PyNodeWrapper<ExchangeViewNode>(std::move(node));
	}

	//============================================================================
	ATLAS_API void setFilter(ExchangeViewFilterType type, double value) noexcept
	{
		m_filter = ExchangeViewFilter(type, value);
	}



	[[nodiscard]] AssetNodeType getType() const noexcept { return m_asset_op_node.t; }
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] size_t getViewSize() const noexcept { return m_view_size; }
	[[nodiscard]] auto& getExchange() { return m_exchange; }
	void filter(Eigen::VectorXd& v) const noexcept;

	ATLAS_API  void evaluate(Eigen::VectorXd&) noexcept override;
};


};

}