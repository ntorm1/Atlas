module;
module PortfolioModule;


namespace Atlas
{

class PortfolioImpl
{
public:
	double initial_cash;
	PortfolioImpl(
		Exchange& exchange,
		double cash
	) noexcept
		: initial_cash(cash)
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


}