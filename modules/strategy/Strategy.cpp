module;
#define NOMINMAX
#include <Eigen/Dense>
#include <iostream>
module StrategyModule;

import ExchangeModule;
import TracerModule;
import PortfolioModule;
import StrategyNodeModule;

import AtlasLinAlg;


namespace Atlas
{

//============================================================================
class StrategyImpl
{
public:
	Exchange& m_exchange;
	Portfolio& m_portfolio;
	Tracer m_tracer;
	Eigen::VectorXd m_target_weights_buffer;
	Eigen::VectorXd m_adjustment_buffer;
	SharedPtr<AST::StrategyNode> m_ast;
	double m_alloc_epsilon = 0.0;

	StrategyImpl(
		Strategy* strategy,
		SharedPtr<AST::StrategyNode> ast,
		double cash
	) noexcept :
		m_portfolio(ast->getPortfolio()),
		m_exchange(ast->getExchange()),
		m_tracer(*strategy, m_exchange, cash),
		m_ast(std::move(ast))
	{
		m_exchange.registerStrategy(strategy);
		m_target_weights_buffer.resize(m_exchange.getAssetCount());
		m_adjustment_buffer.resize(m_exchange.getAssetCount());
		m_alloc_epsilon = m_ast->getAllocEpsilon();

		m_target_weights_buffer.setZero();
		m_adjustment_buffer.setZero();
	}
};


//============================================================================
Strategy::Strategy(
	String name,
	SharedPtr<AST::StrategyNode> ast,
	double portfolio_weight
) noexcept
{
	double init_cash = ast->getPortfolio().getInitialCash();
	m_impl = std::make_unique<StrategyImpl>(
		this,
		std::move(ast),
		portfolio_weight * init_cash
	);
	m_name = std::move(name);
}


//============================================================================
Strategy::~Strategy() noexcept
{
}


//============================================================================
Eigen::VectorXd const&
Strategy::getAllocationBuffer() const noexcept
{
	return m_impl->m_target_weights_buffer;
}


//============================================================================
double
Strategy::getAllocation(size_t asset_index) const noexcept
{
	assert(asset_index < static_cast<size_t>(m_impl->m_target_weights_buffer.rows()));
	return m_impl->m_target_weights_buffer[asset_index];
}


//============================================================================
Tracer const&
	Strategy::getTracer() const noexcept
{
	return m_impl->m_tracer;
}


//============================================================================
double
Strategy::getNLV() const noexcept
{
	return m_impl->m_tracer.getNLV();
}


//============================================================================
void
Strategy::enableTracerHistory(TracerType t) noexcept
{
	m_impl->m_tracer.enableTracerHistory(t);
}


//============================================================================
Eigen::VectorXd const&
Strategy::getHistory(TracerType t) const noexcept
{
	return m_impl->m_tracer.getHistory(t);
}


//============================================================================
Eigen::MatrixXd const&
Strategy::getWeightHistory() const noexcept
{
	return m_impl->m_tracer.m_weight_history;
}


//============================================================================
Exchange const&
Strategy::getExchange() const noexcept
{
	return m_impl->m_exchange;
}


//============================================================================
void
Strategy::evaluate() noexcept
{
	// get the current market returns
	LinAlg::EigenConstColView market_returns = m_impl->m_exchange.getMarketReturns();

	// get the portfolio return by calculating the sum product of the market returns and the portfolio weights
	assert(market_returns.rows() == m_impl->m_target_weights_buffer.rows());
	assert(!market_returns.array().isNaN().any());
	
	// print out target weights buffer and market returns
	double portfolio_return = market_returns.dot(m_impl->m_target_weights_buffer);

	// update the tracer nlv 
	double nlv = m_impl->m_tracer.getNLV();
	m_impl->m_tracer.setNLV(nlv * (1.0 + portfolio_return));
	m_impl->m_tracer.evaluate();
}


//============================================================================
void
Strategy::lateRebalance() noexcept
{
	// if the strategy does not override the target weights buffer at the end of a time step,
	// then we need to rebalance the portfolio to the target weights buffer according to the market returns
	LinAlg::EigenConstColView market_returns = m_impl->m_exchange.getMarketReturns();

	// update the target weights buffer according to the indivual asset returns
	Eigen::VectorXd returns = market_returns.array() + 1.0;
	m_impl->m_target_weights_buffer = returns.cwiseProduct(m_impl->m_target_weights_buffer);

	// divide the target weights buffer by the sum to get the new weights
	auto sum = m_impl->m_target_weights_buffer.sum();
	if (sum > 0.0)
	{
		m_impl->m_target_weights_buffer /= sum;
	}
	assert(!m_impl->m_target_weights_buffer.array().isNaN().any());
}


//============================================================================
void
Strategy::step() noexcept
{
	if (!m_step_call)
	{
		return;
	}
	// check if warmup over 
	if (m_impl->m_exchange.currentIdx() < m_impl->m_ast->getWarmup())
	{
		return;
	}
	// execute the strategy AST node. Update rebalance call if AST 
	// did not update the target weights buffer
	if (!m_impl->m_ast->evaluate(m_impl->m_target_weights_buffer))
	{
		m_late_rebalance_call = true;
	}
	else
	{
		m_late_rebalance_call = false;
	}
	assert(!m_impl->m_target_weights_buffer.array().isNaN().any());
	m_step_call = false;
}


//============================================================================
void
Strategy::reset() noexcept
{
	m_impl->m_tracer.reset();
	m_impl->m_target_weights_buffer.setZero();
	m_impl->m_adjustment_buffer.setZero();
	m_impl->m_ast->reset();
	m_step_call = false;
}


}