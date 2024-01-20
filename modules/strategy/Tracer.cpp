module;
#include <Eigen/Dense>
module TracerModule;

import ExchangeModule;

namespace Atlas
{


//============================================================================
Tracer::Tracer(Exchange const& exchange, double cash
) noexcept : 
    m_exchange(exchange)
{
    m_cash = cash;
    m_nlv = cash;
    m_initial_cash = cash;
	m_data.resize(exchange.getAssetCount(), 1);
    m_data.setZero();
}


//============================================================================
void
Tracer::evaluate() noexcept
{
    if (m_nlv_history.size() > 0) {
		m_nlv_history(m_idx) = m_nlv;
	}
    m_idx++;
}


//============================================================================
void
Tracer::reset() noexcept
{
    m_cash = m_initial_cash;
	m_nlv = m_initial_cash;
	m_data.setZero();

    if (m_nlv_history.size() > 0) {
        m_nlv_history.setZero();
    }

    m_idx = 0;
}


//============================================================================
void
Tracer::enableTracerHistory(TracerType t) noexcept
{
    size_t n = m_exchange.getTimestamps().size();
    switch (t) {
        case TracerType::NLV:
			m_nlv_history.resize(n);
			m_nlv_history.setZero();
			break;
    }
}


//============================================================================
Eigen::VectorXd const&
Tracer::getHistory(TracerType t) const noexcept
{
    switch (t) {
		case TracerType::NLV:
			return m_nlv_history;
	}
}



}