module;
module PortfolioModule;

import ExchangeModule;

namespace Atlas
{

class PortfolioImpl
{
public:
	double initial_cash;
	Exchange& exchange;
	PortfolioImpl(
		Exchange& exchange,
		double cash
	) noexcept
		: initial_cash(cash), exchange(exchange)
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
double
Portfolio::getInitialCash() const noexcept
{
	return m_impl->initial_cash;
}


//============================================================================
String const&
Portfolio::getExchangeName() const noexcept
{
	return m_impl->exchange.getName();
}


}