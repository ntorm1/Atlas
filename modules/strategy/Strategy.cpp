module;
#define NOMINMAX
#include <Eigen/Dense>
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
	UniquePtr<AST::StrategyNode> m_ast;
	double m_alloc_epsilon = 0.0;

	StrategyImpl(
		Strategy* strategy,
		UniquePtr<AST::StrategyNode> ast,
		double cash
	) noexcept :
		m_ast(std::move(ast)),
		m_portfolio(ast->getPortfolio()),
		m_exchange(ast->getExchange()),
		m_tracer(m_exchange, cash)
	{
		m_exchange.registerStrategy(strategy);
		m_target_weights_buffer.resize(m_exchange.getAssetCount());
		m_adjustment_buffer.resize(m_exchange.getAssetCount());
		m_alloc_epsilon = m_ast->getAllocEpsilon();
	}
};


//============================================================================
Strategy::Strategy(
	String name,
	UniquePtr<AST::StrategyNode> ast,
	double portfolio_weight
) noexcept
{
	double init_cash = ast->getPortfolio().getTracer().getInitialCash();
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
Tracer const&
Strategy::getTracer() const noexcept
{
	return m_impl->m_tracer;
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
	double portfolio_return = market_returns.dot(m_impl->m_target_weights_buffer);

	// update the tracer nlv 
	double nlv = m_impl->m_tracer.getNLV();
	m_impl->m_tracer.setNLV(nlv * (1.0 + portfolio_return));
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
	m_impl->m_ast->evaluate(m_impl->m_target_weights_buffer);
	assert(!m_impl->m_target_weights_buffer.array().isNaN().any());

	m_step_call = false;
}


}