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
ExchangeViewNode::~ExchangeViewNode() noexcept
{
}


//============================================================================
ExchangeViewNode::ExchangeViewNode(
    Exchange& exchange,
    SharedPtr<StrategyBufferOpNode> asset_op_node
) noexcept :
    StrategyBufferOpNode(NodeType::EXCHANGE_VIEW, exchange, std::nullopt),
    m_asset_op_node(std::move(asset_op_node)),
    m_exchange(exchange),
    m_warmup(m_asset_op_node->getWarmup())
{
    m_view_size = m_exchange.getAssetCount();
}


//============================================================================
ExchangeViewNode::ExchangeViewNode(
    SharedPtr<Exchange> exchange,
    SharedPtr<StrategyBufferOpNode> asset_op_node
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
    m_asset_op_node->evaluate(view);
    if (m_filter.has_value()) {
        filter(view);
    }
}

}

}