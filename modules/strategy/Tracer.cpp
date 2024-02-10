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
            m_volatility_history(m_idx) = std::sqrt(current_vol(0)) * std::sqrt(252);
        }
	}
    m_idx++;
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
    }
    return true;
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
}



}