module;
module MeasureModule;

import ExchangeModule;

namespace Atlas {

Measure::Measure(TracerType type, Exchange const &exchange) noexcept
    : m_exchange(&exchange), m_type(type),
      m_values(LinAlg::EigenVectorXd::Zero(exchange.getAssetCount())) {}

Measure::~Measure() noexcept {}

} // namespace Atlas