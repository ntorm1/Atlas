module;
#include <cassert>
#include "AtlasMacros.hpp"
#include <stdexcept>
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
	Vector<SharedPtr<Strategy>> m_strategies;
	Vector<SharedPtr<Portfolio>> m_portfolios;
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
ExchangeMap const&
Hydra::getExchangeMap() const noexcept
{
	return m_impl->m_exchange_map;
}


//============================================================================
Result<SharedPtr<Exchange>, AtlasException>
Hydra::addExchange(String name, String source) noexcept
{
	if (m_state != HydraState::INIT && m_state != HydraState::BUILT)
	{
		return Err("Hydra must be in init state to add exchange");
	}
	auto res = m_impl->m_exchange_map.addExchange(std::move(name), std::move(source));
	if (!res)
	{
		return res;
	}
	m_state = HydraState::INIT;
	return res;
}


//============================================================================
Result<Exchange*, AtlasException>
Hydra::getExchange(String const& name) const noexcept
{
	return m_impl->m_exchange_map.getExchange(name);
}


//============================================================================
Result<Strategy const*, AtlasException>
Hydra::addStrategy(
	SharedPtr<Strategy> strategy,
	bool replace_if_exists) noexcept
{
	if (m_state != HydraState::BUILT)
	{
		return Err("Hydra must be in build state to add strategy");
	}
	if (m_impl->m_strategy_map.contains(strategy->getName()))
	{
		if (!replace_if_exists)
		{
			return Err("Strategy with name " + strategy->getName() + " already exists");

		}
		// find the strategy and replace it in the vector
		auto idx = m_impl->m_strategy_map[strategy->getName()];
		m_impl->m_strategies[idx] = std::move(strategy);
		return m_impl->m_strategies[idx].get();
	}
	strategy->setID(m_impl->m_strategies.size());
	m_impl->m_strategy_map[strategy->getName()] = m_impl->m_strategies.size() - 1;
	m_impl->m_strategies.push_back(std::move(strategy));
	return m_impl->m_strategies.back().get();
}


//============================================================================
Result<SharedPtr<Portfolio>, AtlasException>
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
	auto portfolio = std::make_shared<Portfolio>(
		std::move(name), m_impl->m_portfolio_map.size(), exchange, initial_cash
	);	
	m_impl->m_portfolios.push_back(portfolio);
	m_impl->m_portfolio_map[portfolio->getName()] = portfolio->getId();
	return portfolio;
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
Hydra::removeStrategy(String const& name) noexcept
{
	if (!m_impl->m_strategy_map.contains(name))
	{
		return;
	}
	auto idx = m_impl->m_strategy_map[name];
	// update all the strategy ids greater than the one we are removing
	for (size_t i = idx + 1; idx < m_impl->m_strategies.size() - 1; ++i)
	{
		m_impl->m_strategies[i]->setID(i-1);
		m_impl->m_strategy_map[m_impl->m_strategies[i]->getName()] = i-1;
	}
	// erase the strategy from the vector
	m_impl->m_strategies.erase(m_impl->m_strategies.begin() + idx);
	// update the strategy map
	m_impl->m_strategy_map.erase(name);
}


//============================================================================
void
Hydra::step() noexcept
{
	assert(m_state == HydraState::BUILT || m_state == HydraState::RUNING);
	m_impl->m_exchange_map.step();
	
	for (int i = 0; i < m_impl->m_strategies.size(); i++)
	{
		auto& strategy = m_impl->m_strategies[i];
		// evaluate the strategy with the current target weights
		strategy->evaluate();
		// execute strategy logic to populate new target weights
		strategy->step();
		// if no action was taken, propogate asset returns to adjust weights
		if (strategy->m_late_rebalance_call)
		{
			strategy->lateRebalance();
		}
	}
}



//============================================================================
void
Hydra::run() noexcept
{
	assert(m_state == HydraState::BUILT || m_state == HydraState::FINISHED);
	if (m_state == HydraState::FINISHED)
	{
		auto res = reset();
		assert(res);
	}
	size_t steps = m_impl->m_exchange_map.getTimestamps().size();
	for (size_t i = 0; i < steps; ++i)
	{
		step();
	}
	m_state = HydraState::FINISHED;
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
	m_state = HydraState::BUILT;
	return true;
}


//============================================================================
SharedPtr<Exchange>
Hydra::pyAddExchange(String name, String source)
{
	auto res = addExchange(std::move(name), std::move(source));
	if (!res)
	{
		throw std::exception(res.error().what());
	}
	return *res;
}


//============================================================================
SharedPtr<Portfolio>
Hydra::pyAddPortfolio(String name, SharedPtr<Exchange> exchange, double intial_cash)
{
	auto res = addPortfolio(std::move(name), *(exchange.get()), intial_cash);
	if (!res)
	{
		throw std::exception(res.error().what());
	}
	return *res;
}


//============================================================================
SharedPtr<Strategy>
Hydra::pyAddStrategy(
	SharedPtr<Strategy> strategy,
	bool replace_if_exists)
{
	auto res = addStrategy(std::move(strategy), replace_if_exists);
	if (!res)
	{
		throw std::exception(res.error().what());
	}
	return m_impl->m_strategies.back();
}


//============================================================================
void
Hydra::pyBuild()
{
	auto res = build();
	if (!res)
	{
		throw std::exception(res.error().what());
	}
}


//============================================================================
void
Hydra::pyReset()
{
	auto res = reset();
	if (!res)
	{
		throw std::exception(res.error().what());
	}
}


}