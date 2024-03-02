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
        asSignal(true);
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
bool
ExchangeViewNode::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
    return false; //TODO 
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
    if (m_take_from_cache)
    {
		target = cacheColumn();
		return;
	}

    assert(static_cast<size_t>(target.rows()) == getExchange().getAssetCount());
    assert(static_cast<size_t>(target.cols()) == 1);

    // on evaluation start ev is pass in the current weights to override.
    // store these weights to compare agaisnt the next time step

    m_asset_op_node->evaluate(target);
    if (m_filters.size()) {
        filter(target);
    }

    // If operating ev as a signal, then we assume that the left and right view cannot be non-Nan
    // at the same time. We only update the EV when the opposing view is non-Nan.
    if (m_as_signal && m_left_view)
    {
        // copy over the previous ev signals to a temp buffer
        LinAlg::EigenVectorXd temp = m_buffer;

        // evaluate the left view over the ev buffer
        (*m_left_view)->evaluate(m_buffer);

        for (int i = 0; i < m_buffer.size(); i++)
		{
            // if left signal not nan take that
            if (!std::isnan(m_buffer(i)))
            {
                if (m_buffer(i) * temp(i) > 0)
                {
                    target(i) = temp(i);
                }
                else
                {
                    target(i) = m_buffer(i);
                }
            }
            // else if previous signal not nan take that. NOTE: assume that if the main ev signal
            // evaluates to NaN we still are in the position untill the right signal fires.
            else if (!std::isnan(temp(i)))
			{
                if (target(i) * temp(i) > 0 || std::isnan(target(i)))
                {
                    target(i) = temp(i);
                }
			}
		}
        m_buffer = target;
    }
    if (hasCache())
        cacheColumn() = target;
}

}

}