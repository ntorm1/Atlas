module;
#include <Eigen/Dense>
#include "AtlasMacros.hpp"
module TracerModule;

import ExchangeModule;
import StrategyModule;
import RiskNodeModule;

namespace Atlas
{


//============================================================================
Tracer::Tracer(
    Strategy const& strategy,
    Exchange const& exchange,
    double cash
) noexcept : 
    m_exchange(exchange),
    m_strategy(strategy)
{
    m_cash = cash;
    m_nlv = cash;
    m_initial_cash = cash;
    m_weight_history.resize(0, 0);
    m_nlv_history.resize(0);
    m_weights_buffer.resize(exchange.getAssetCount());
    m_weights_buffer.setZero();
}


//============================================================================
void
Tracer::evaluate() noexcept
{
    if (m_nlv_history.size() > 0) {
		m_nlv_history(m_idx) = m_nlv;
	}
    if (m_weight_history.cols() > 0) 
    {
		m_weight_history.col(m_idx) = m_strategy.getAllocationBuffer();
    }
    if (m_volatility_history.size() > 0) 
	{
        assert(m_covariance);
        if (m_idx > (*m_covariance)->getWarmup())
        {
            LinAlg::EigenMatrixXd const& covariance = (*m_covariance)->getCovariance();
            LinAlg::EigenVectorXd const& weights = m_strategy.getAllocationBuffer();
            auto current_vol = (weights.transpose() * covariance * weights);
            double current_volatility = std::sqrt(current_vol(0)) * std::sqrt(252);
            m_volatility_history(m_idx) = current_volatility;
        }
	}
    if (m_struct_tracer && m_struct_tracer.value()->eager())
    {
        m_struct_tracer.value()->evaluate(
			m_strategy.getAllocationBuffer(),
			m_weights_buffer
		);
    }
    m_idx++;
}


//============================================================================
void
Tracer::initPnL() noexcept
{
    m_pnl.resize(m_exchange.getAssetCount());
    m_pnl.setZero();
}


//============================================================================
LinAlg::EigenVectorXd& 
Tracer::getPnL() noexcept
{
    if (!m_pnl.rows()) 
    {
		initPnL();
	}
    return m_pnl;
}


//============================================================================
void
Tracer::reset() noexcept
{
    m_cash = m_initial_cash;
	m_nlv = m_initial_cash;

    if (m_nlv_history.size() > 0) 
    {
        m_nlv_history.setZero();
    }
    if (m_weight_history.cols() > 0) 
    {
		m_weight_history.setZero();
	}
    if (m_volatility_history.size() > 0) 
	{
        m_volatility_history.setZero();
	}
    if (m_struct_tracer)
    {
        m_struct_tracer.value()->reset();
    }
    m_weights_buffer.setZero();
    m_pnl.setZero();
    m_idx = 0;
}


//============================================================================
Result<bool, AtlasException>
Tracer::enableTracerHistory(TracerType t) noexcept
{
    size_t n = m_exchange.getTimestamps().size();
    switch (t) {
        case TracerType::NLV:
			m_nlv_history.resize(n);
			m_nlv_history.setZero();
			break;
        case TracerType::WEIGHTS:
            m_weight_history.resize(m_exchange.getAssetCount(), n);
            m_weight_history.setZero();
            break;
        case TracerType::VOLATILITY:
            if (!m_covariance) {
                return Err("Covariance matrix not set");
            }
            m_volatility_history.resize(n);
			m_volatility_history.setZero();
			break;
        case TracerType::ORDERS_LAZY:
        case TracerType::ORDERS_EAGER:
            if (!m_struct_tracer) {
                m_struct_tracer = std::make_unique<StructTracer>(m_exchange, *this);
            }
            m_struct_tracer.value()->enabelTracerHistory(t);
            break;
    }
    return true;
}


//============================================================================
void
Tracer::setPnL(LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl) noexcept
{
    m_pnl = pnl;
}


//============================================================================
void
Tracer::realize() noexcept
{
    if (m_struct_tracer) {
		m_struct_tracer.value()->realize();
	}
}


//============================================================================
Eigen::VectorXd const&
Tracer::getHistory(TracerType t) const noexcept
{
    switch (t) {
		case TracerType::NLV:
			return m_nlv_history;
        case TracerType::VOLATILITY:
            return m_volatility_history;
        case TracerType::WEIGHTS:
            assert(false);
            break; // handled differently 
	}
    std::unreachable();
}


//============================================================================
StructTracer::StructTracer(
    Exchange const& exchange,
    Tracer const& tracer
) noexcept:
    m_exchange(exchange),
    m_tracer(tracer)
{
    auto close_idx = exchange.getCloseIndex();
    assert(close_idx);
    close_index = *close_idx;
}

//============================================================================
StructTracer::~StructTracer() noexcept
{

}


//============================================================================
void
StructTracer::enabelTracerHistory(TracerType t) noexcept
{
	switch (t) {
		case TracerType::ORDERS_EAGER:
            orders_eager = true;
			break;
        case TracerType::ORDERS_LAZY:
            orders_lazy = true;
			break;
        default:
            return;
	}
}


//============================================================================
void
StructTracer::evaluate(
    LinAlg::EigenVectorXd const& weights,
    LinAlg::EigenVectorXd const& previous_weights
) noexcept
{
    if (orders_eager) {
        // get the deviation from the previous weights
        LinAlg::EigenVectorXd deviation = weights - previous_weights;
        // scale weights by the nlv 
        deviation *= m_tracer.m_nlv;
        // fetch the current market prices
        auto close_prices = m_exchange.getSlice(close_index, 0);
        // scale the deviation by the market prices to get the units
        deviation = deviation.cwiseQuotient(close_prices);
        // populate order struct where deviation is greater than 0
        Int64 current_time = m_exchange.getCurrentTimestamp();
        size_t exchange_offset = m_exchange.getExchangeOffset();
        size_t strategy_id = m_tracer.m_strategy.getId();
        for (size_t i = 0; i < static_cast<size_t>(deviation.size()); i++) {
            auto order = Order(
				i + exchange_offset,
				strategy_id,
				current_time,
				deviation(i),
				close_prices(i)
			);
            m_orders.push_back(std::move(order));
        }
    }
}


//============================================================================
void
StructTracer::realize() noexcept
{
    if (!m_tracer.m_weight_history.cols()) return;

    if (orders_lazy) {
        for (size_t i = 1; i < static_cast<size_t>(m_tracer.m_weight_history.cols()); i++) {
            auto previous_weights = m_tracer.m_weight_history.col(i - 1);
            auto current_weights = m_tracer.m_weight_history.col(i);
            evaluate(current_weights, previous_weights);
        }
    }
}


//============================================================================
void
StructTracer::reset() noexcept
{
    m_orders.clear();
    m_trades.clear();
}



}