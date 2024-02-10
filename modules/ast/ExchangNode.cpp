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
    for (auto const& filter : m_filters)
    {
        double c = filter->value;
        Option<double> value_inplace = filter->value_inplace;
        switch (filter->type) {
        case ExchangeViewFilterType::GREATER_THAN:
            if (value_inplace.has_value())
                view = (view.array() > c).select(value_inplace.value() * Eigen::VectorXd::Ones(view.size()), view);
            else
                view = (view.array() > c).select(view, std::numeric_limits<double>::quiet_NaN());
            break;
        case ExchangeViewFilterType::LESS_THAN:
            if (value_inplace.has_value())
                view = (view.array() < c).select(value_inplace.value() * Eigen::VectorXd::Ones(view.size()), view);
            else
                view = (view.array() < c).select(view, std::numeric_limits<double>::quiet_NaN());
            break;
        case ExchangeViewFilterType::EQUAL_TO:
            if (value_inplace.has_value())
                view = (view.array().abs() == c).select(value_inplace.value() * Eigen::VectorXd::Ones(view.size()), view);
            else
                view = (view.array().abs() == c).select(view, std::numeric_limits<double>::quiet_NaN());
            break;
        }
    }
}

//============================================================================
void
    ExchangeViewNode::evaluate(Eigen::VectorXd& view) noexcept
{
    m_asset_op_node->evaluate(view);
    if (m_filters.size()) {
        filter(view);
    }
}

}

}