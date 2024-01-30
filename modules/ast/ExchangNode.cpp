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
AssetOpNodeVariant::AssetOpNodeVariant(
    const AssetOpNodeVariant& other
):
    warmup(other.warmup),
    t(other.t)
{
    switch (t)
    {
    case AssetNodeType::AssetReadNode:
        value = std::move(std::get<0>(other.value));
        break;
    case AssetNodeType::AssetProductNode:
        value = std::move(std::get<1>(other.value));
        break;
    case AssetNodeType::AssetQuotientNode:
        value = std::move(std::get<2>(other.value));
        break;
    case AssetNodeType::AssetSumNode:
        value = std::move(std::get<3>(other.value));
        break;
    case AssetNodeType::AssetDifferenceNode:
        value = std::move(std::get<4>(other.value));
        break;
    default:
        assert(false);
    }
}


//============================================================================
AssetOpNodeVariant::AssetOpNodeVariant(node_variant node)
    noexcept :
    value(std::move(node)),
    warmup(0),
    t(AssetNodeType::AssetReadNode)
{
    if (std::holds_alternative<UniquePtr<AssetReadNode>>(value)) {
        warmup = std::get<UniquePtr<AssetReadNode>>(value)->getWarmup();
        t = AssetNodeType::AssetReadNode;
    }
    else if (std::holds_alternative<UniquePtr<AssetProductNode>>(value)) {
        warmup = std::get<UniquePtr<AssetProductNode>>(value)->getWarmup();
        t = AssetNodeType::AssetProductNode;
    }
    else if (std::holds_alternative<UniquePtr<AssetQuotientNode>>(value)) {
        warmup = std::get<UniquePtr<AssetQuotientNode>>(value)->getWarmup();
        t = AssetNodeType::AssetQuotientNode;
    }
    else if (std::holds_alternative<UniquePtr<AssetSumNode>>(value)) {
        warmup = std::get<UniquePtr<AssetSumNode>>(value)->getWarmup();
        t = AssetNodeType::AssetSumNode;
    }
    else if (std::holds_alternative<UniquePtr<AssetDifferenceNode>>(value)) {
        warmup = std::get<UniquePtr<AssetDifferenceNode>>(value)->getWarmup();
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
    StrategyBufferOpNode(NodeType::EXCHANGE_VIEW, exchange, std::nullopt),
    m_asset_op_node(std::move(asset_op_node)),
    m_exchange(exchange),
    m_warmup(m_asset_op_node.warmup)
{
    m_view_size = m_exchange.getAssetCount();
}


//============================================================================
ExchangeViewNode::ExchangeViewNode(
    SharedPtr<Exchange> exchange,
    AssetOpNodeVariant asset_op_node
) noexcept :
    ExchangeViewNode(*exchange, std::move(asset_op_node)) 
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
        view = std::get<UniquePtr<AssetReadNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetProductNode:
        view = std::get<UniquePtr<AssetProductNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetQuotientNode:
        view = std::get<UniquePtr<AssetQuotientNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetSumNode:
        view = std::get<UniquePtr<AssetSumNode>>(m_asset_op_node.value)->evaluate();
        break;
    case AssetNodeType::AssetDifferenceNode:
        view = std::get<UniquePtr<AssetDifferenceNode>>(m_asset_op_node.value)->evaluate();
        break;
    }
    if (m_filter.has_value()) {
        filter(view);
    }
}

}

}