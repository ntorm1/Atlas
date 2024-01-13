module;
#include <Eigen/Dense>
#include <iostream>
module ExchangeNodeModule;

import ExchangeModule;
import AssetNodeModule;

namespace Atlas
{

namespace AST

{

//============================================================================
AssetOpNodeVariant::AssetOpNodeVariant(node_variant node) 
noexcept :
    value(std::move(node)),
    warmup(0),
    t(AssetNodeType::AssetReadNode)
{
    if (std::holds_alternative<std::unique_ptr<AssetReadNode>>(value)) {
        warmup = std::get<std::unique_ptr<AssetReadNode>>(value)->getWarmup();
        t = AssetNodeType::AssetReadNode;
    }
    else if (std::holds_alternative<std::unique_ptr<AssetProductNode>>(value)) {
        warmup = std::get<std::unique_ptr<AssetProductNode>>(value)->getWarmup();
        t = AssetNodeType::AssetProductNode;
    }
    else if (std::holds_alternative<std::unique_ptr<AssetQuotientNode>>(value)) {
        warmup = std::get<std::unique_ptr<AssetQuotientNode>>(value)->getWarmup();
        t = AssetNodeType::AssetQuotientNode;
    }
    else if (std::holds_alternative<std::unique_ptr<AssetSumNode>>(value)) {
        warmup = std::get<std::unique_ptr<AssetSumNode>>(value)->getWarmup();
        t = AssetNodeType::AssetSumNode;
    }
    else if (std::holds_alternative<std::unique_ptr<AssetDifferenceNode>>(value)) {
        warmup = std::get<std::unique_ptr<AssetDifferenceNode>>(value)->getWarmup();
        t = AssetNodeType::AssetDifferenceNode;
    }
}


//============================================================================
AssetOpNodeVariant::~AssetOpNodeVariant() noexcept 
{
}


//============================================================================
ExchangeViewNode::~ExchangeViewNode() noexcept
{
}


//============================================================================
ExchangeViewNode::ExchangeViewNode(
	Exchange& exchange,
	AssetOpNodeVariant asset_op_node,
    Option<ExchangeViewFilter> m_filter
) noexcept :
    OpperationNode<void, Eigen::VectorXd&>(NodeType::EXCHANGE_VIEW),
	m_asset_op_node(std::move(asset_op_node)),
	m_exchange(exchange),
    m_warmup(m_asset_op_node.warmup),
    m_filter(std::move(m_filter))
{
}


//============================================================================
void
ExchangeViewNode::filter(Eigen::VectorXd& view) const noexcept
{
    assert(m_filter.has_value());
    double c = m_filter.value().value;
    switch ((*m_filter).type) {
        case ExchangeViewFilterType::GREATER_THAN:
            view = (view.array() > c).select(view.array(), std::numeric_limits<double>::quiet_NaN());
		    break;
    }
}

//============================================================================
void
ExchangeViewNode::evaluate(Eigen::VectorXd& view) noexcept
{
    switch (m_asset_op_node.t) {
    case AssetNodeType::AssetReadNode:
        view = std::get<std::unique_ptr<AssetReadNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetProductNode:
        view = std::get<std::unique_ptr<AssetProductNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetQuotientNode:
        view = std::get<std::unique_ptr<AssetQuotientNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetSumNode:
        view = std::get<std::unique_ptr<AssetSumNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetDifferenceNode:
        view = std::get<std::unique_ptr<AssetDifferenceNode>>(m_asset_op_node.value)->evaluate();
        break;
    }
    if (m_filter.has_value()) {
		filter(view);
	}
}

}

}