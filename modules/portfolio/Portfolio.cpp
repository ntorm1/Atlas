module;
module PortfolioModule;

import TracerModule;

namespace Atlas
{

class PortfolioImpl
{
public:
	Tracer m_tracer;

	PortfolioImpl(
		Exchange& exchange,
		double cash
	) noexcept
		: m_tracer(exchange, cash)
	{

	}
};


//============================================================================
Portfolio::Portfolio(
	String name,
	size_t id,
	Exchange& exchange,
	double initial_cash
) noexcept
	: m_name(std::move(name)), m_id(id)
{
	m_impl = std::make_unique<PortfolioImpl>(exchange, initial_cash);
}


//============================================================================
Portfolio::~Portfolio() noexcept
{
}


//============================================================================
Tracer const& Portfolio::getTracer() const noexcept
{
	return m_impl->m_tracer;
}

}