module;
#include <Eigen/Dense>
module TracerModule;

import ExchangeModule;

namespace Atlas
{


//============================================================================
Tracer::Tracer(Exchange const& exchange, double cash) noexcept
{
    m_cash = cash;
    m_nlv = cash;
    m_initial_cash = cash;
	m_data.resize(exchange.getAssetCount(), 1);
    setTracerFlag(TracerItem::ALLOC_PCT);
}


//============================================================================
bool
Tracer::isTracerFlagSet(TracerItem flag) const noexcept
{
	return (m_flags & static_cast<uint8_t>(flag)) != 0;
}


//============================================================================
void
Tracer::clearTracerFlag(TracerItem flag) noexcept
{
    m_flags &= ~static_cast<uint8_t>(flag);
}


//============================================================================
void
Tracer::setTracerFlag(TracerItem flag) noexcept
{
    m_flags |= static_cast<uint8_t>(flag);
}


//============================================================================
size_t
Tracer::getTracerCount() const noexcept
{
    // Count the number of set bits in 'flags'
    int count = 0;
    uint8_t tempFlags = m_flags;

    while (tempFlags) {
        count += tempFlags & 1;
        tempFlags >>= 1;
    }
    return count;
}


//============================================================================
void Tracer::evaluate(bool is_reprice) noexcept
{
}



}