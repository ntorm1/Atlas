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
    if (std::holds_alternative<SharedPtr<AssetReadNode>>(value)) {
        warmup = std::get<SharedPtr<AssetReadNode>>(value)->getWarmup();
        t = AssetNodeType::AssetReadNode;
    }
    else if (std::holds_alternative<SharedPtr<AssetProductNode>>(value)) {
        warmup = std::get<SharedPtr<AssetProductNode>>(value)->getWarmup();
        t = AssetNodeType::AssetProductNode;
    }
    else if (std::holds_alternative<SharedPtr<AssetQuotientNode>>(value)) {
        warmup = std::get<SharedPtr<AssetQuotientNode>>(value)->getWarmup();
        t = AssetNodeType::AssetQuotientNode;
    }
    else if (std::holds_alternative<SharedPtr<AssetSumNode>>(value)) {
        warmup = std::get<SharedPtr<AssetSumNode>>(value)->getWarmup();
        t = AssetNodeType::AssetSumNode;
    }
    else if (std::holds_alternative<SharedPtr<AssetDifferenceNode>>(value)) {
        warmup = std::get<SharedPtr<AssetDifferenceNode>>(value)->getWarmup();
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
    AssetOpNodeVariant asset_op_node
) noexcept :
    OpperationNode<void, Eigen::VectorXd&>(NodeType::EXCHANGE_VIEW),
    m_asset_op_node(std::move(asset_op_node)),
    m_exchange(exchange),
    m_warmup(m_asset_op_node.warmup)
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
    case ExchangeViewFilterType::LESS_THAN:
		view = (view.array() < c).select(view.array(), std::numeric_limits<double>::quiet_NaN());
		break;
    case ExchangeViewFilterType::EQUAL_TO:
        view = (view.array().abs() - c).select(view.array(), std::numeric_limits<double>::quiet_NaN());
        break;
    }
}

//============================================================================
void
    ExchangeViewNode::evaluate(Eigen::VectorXd& view) noexcept
{
    switch (m_asset_op_node.t) {
    case AssetNodeType::AssetReadNode:
        view = std::get<SharedPtr<AssetReadNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetProductNode:
        view = std::get<SharedPtr<AssetProductNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetQuotientNode:
        view = std::get<SharedPtr<AssetQuotientNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetSumNode:
        view = std::get<SharedPtr<AssetSumNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetDifferenceNode:
        view = std::get<SharedPtr<AssetDifferenceNode>>(m_asset_op_node.value)->evaluate();
        break;
    }
    if (m_filter.has_value()) {
        filter(view);
    }
}

}

}