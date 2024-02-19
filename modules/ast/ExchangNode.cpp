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
    SharedPtr<Exchange> exchange,
    SharedPtr<StrategyBufferOpNode> asset_op_node,
    Option<SharedPtr<ExchangeViewNode>> left_view
) noexcept :
    StrategyBufferOpNode(NodeType::EXCHANGE_VIEW, *exchange, std::nullopt),
    m_asset_op_node(std::move(asset_op_node)),
    m_warmup(m_asset_op_node->getWarmup()),
    m_left_view(std::move(left_view))
{
    m_view_size = m_exchange.getAssetCount();
    if (m_left_view)
    {
        m_buffer.resize(m_exchange.getAssetCount());
        m_buffer.setZero();
    }
    else
    {
        m_buffer.resize(0);
    }
}


//============================================================================
size_t
ExchangeViewNode::refreshWarmup() noexcept
{
    m_warmup = m_asset_op_node->refreshWarmup();
    return m_warmup;
}


//============================================================================
void
ExchangeViewNode::filter(LinAlg::EigenRef<LinAlg::EigenVectorXd> view) const noexcept
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
ExchangeViewNode::asSignal(bool v) noexcept
{
    if (v && !m_buffer.size())
	{
		m_buffer.resize(m_exchange.getAssetCount());
		m_buffer.setZero();
	}
	m_as_signal = v;
}

//============================================================================
void
ExchangeViewNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
    assert(static_cast<size_t>(target.rows()) == getExchange().getAssetCount());
    assert(static_cast<size_t>(target.cols()) == 1);

    // on evaluation start ev is pass in the current weights to override.
    // store these weights to compare agaisnt the next time step
    if (m_as_signal)
    {
        m_buffer = target;
    }

    m_asset_op_node->evaluate(target);
    if (m_filters.size()) {
        filter(target);
    }

    // if ev is operating as signal compare against previous time step and
    // copy the result to the buffer
    if (m_as_signal)
    {
        assert(m_buffer.size() == target.size());
        target = (
            (target.array() * m_buffer.array() < 0.0f)
            ||
            (m_buffer.array() == 0.0f) && (target.array() != 0.0f)
            )
            .select(target, m_buffer);
    }
}

}

}