module;
#include <cassert>
#include "AtlasMacros.hpp"
module HydraModule;

import ExchangeMapModule;
import PortfolioModule;
import StrategyModule;

namespace Atlas
{

//============================================================================
struct HydraImpl
{
	ExchangeMap m_exchange_map;
	Vector<UniquePtr<Strategy>> m_strategies;
	Vector<UniquePtr<Portfolio>> m_portfolios;
	HashMap<String, size_t> m_strategy_map;
	HashMap<String, size_t> m_portfolio_map;
};


//============================================================================
Hydra::Hydra() noexcept
{
	m_impl = std::make_unique<HydraImpl>();
}


//============================================================================
Hydra::~Hydra() noexcept
{
}


//============================================================================
Result<Exchange*, AtlasException>
Hydra::addExchange(String name, String source) noexcept
{
	return m_impl->m_exchange_map.addExchange(std::move(name), std::move(source));
}


//============================================================================
Result<Exchange*, AtlasException>
Hydra::getExchange(String const& name) const noexcept
{
	return m_impl->m_exchange_map.getExchange(name);
}


//============================================================================
Result<Strategy const*, AtlasException>
Hydra::addStrategy(UniquePtr<Strategy> strategy) noexcept
{
	if (m_state == HydraState::RUNING)
	{
		return Err("Hydra can not be in running to add strategy");
	}
	if (m_impl->m_strategy_map.contains(strategy->getName()))
	{
		return Err("Strategy with name " + strategy->getName() + " already exists");
	}
	strategy->setID(m_impl->m_strategies.size());
	m_impl->m_strategies.push_back(std::move(strategy));
	return m_impl->m_strategies.back().get();
}


//============================================================================
Result<Portfolio*, AtlasException>
Hydra::addPortfolio(String name, Exchange& exchange, double initial_cash) noexcept
{
	if (m_state != HydraState::INIT)
	{
		return Err("Hydra must be in init state to add portfolio");
	}
	if (m_impl->m_portfolio_map.contains(name))
	{
		return Err("Portfolio with name " + name + " already exists");
	}
	auto portfolio = std::make_unique<Portfolio>(
		std::move(name), m_impl->m_portfolio_map.size(), exchange, initial_cash
	);	
	auto result = portfolio.get();
	m_impl->m_portfolios.push_back(std::move(portfolio));
	m_impl->m_portfolio_map[result->getName()] = result->getId();
	return result;
}


//============================================================================
Result<bool, AtlasException>
Hydra::build()
{
	m_impl->m_exchange_map.build();
	m_state = HydraState::BUILT;
	return true;
}


//============================================================================
void
Hydra::step() noexcept
{
	assert(m_state == HydraState::BUILT || m_state == HydraState::RUNING);
	m_impl->m_exchange_map.step();

	for (auto& strategy : m_impl->m_strategies)
	{
		strategy->evaluate();
		strategy->step();
	}
}


//============================================================================
void
Hydra::run() noexcept
{
	assert(m_state == HydraState::BUILT);
	size_t steps = m_impl->m_exchange_map.getTimestamps().size();
	for (size_t i = 0; i < steps; ++i)
	{
		step();
	}
}


//============================================================================
Result<bool, AtlasException>
Hydra::reset() noexcept
{
	if (m_state == HydraState::INIT)
	{
		return Err("Hydra can not be in init state to reset");
	}
	m_impl->m_exchange_map.reset();
	for (auto& strategy : m_impl->m_strategies)
	{
		strategy->reset();
	}
	return true;
}


}