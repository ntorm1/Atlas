module;
#define NOMINMAX
#include <Eigen/Dense>
#include "AtlasMacros.hpp"
module StrategyModule;

import ExchangeModule;
import TracerModule;
import PortfolioModule;
import StrategyNodeModule;
import CommissionsModule;

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
	SharedPtr<AST::StrategyNode> m_ast;
	Option<SharedPtr<CommisionManager>> m_commision_manager;

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
		m_target_weights_buffer.setZero();
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
[[nodiscard]] Result<bool, AtlasException>
Strategy::enableTracerHistory(TracerType t) noexcept
{
	return m_impl->m_tracer.enableTracerHistory(t);
}


//============================================================================
void
Strategy::pyEnableTracerHistory(TracerType t)
{
	auto res = enableTracerHistory(t);
	if (!res)
	{
		throw std::runtime_error(res.error().what());
	}
}


//============================================================================
void
Strategy::setVolTracer(SharedPtr<AST::CovarianceNode> node) noexcept
{
	assert(node);
	m_impl->m_tracer.setCovarianceNode(node);
	auto res = m_impl->m_tracer.enableTracerHistory(TracerType::VOLATILITY);
	assert(res);
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
SharedPtr<CommisionManager>
Strategy::initCommissionManager() noexcept
{
	m_impl->m_commision_manager = CommissionManagerFactory::create(*this);
	m_impl->m_ast->setCommissionManager(m_impl->m_commision_manager.value());
	return m_impl->m_commision_manager.value();
}


//============================================================================
Exchange const&
Strategy::getExchange() const noexcept
{
	return m_impl->m_exchange;
}


//============================================================================
void
Strategy::evaluate(
	Eigen::Ref<Eigen::VectorXd> const& target_weights_buffer
) noexcept
{
	// get the current market returns
	LinAlg::EigenConstColView market_returns = m_impl->m_exchange.getMarketReturns();

	// get the portfolio return by calculating the sum product of the market returns and the portfolio weights
	assert(market_returns.rows() == target_weights_buffer.rows());
	assert(!market_returns.array().isNaN().any());
	
	// print out target weights buffer and market returns
	double portfolio_return = market_returns.dot(target_weights_buffer);

	// update the tracer nlv 
	double nlv = m_impl->m_tracer.getNLV();
	m_impl->m_tracer.setNLV(nlv * (1.0 + portfolio_return));
	m_impl->m_tracer.evaluate();
}


//============================================================================
void
Strategy::lateRebalance(
	Eigen::Ref<Eigen::VectorXd> target_weights_buffer
) noexcept
{
	// if the strategy does not override the target weights buffer at the end of a time step,
	// then we need to rebalance the portfolio to the target weights buffer according to the market returns
	LinAlg::EigenConstColView market_returns = m_impl->m_exchange.getMarketReturns();

	// update the target weights buffer according to the indivual asset returns
	Eigen::VectorXd returns = market_returns.array() + 1.0;
	target_weights_buffer = returns.cwiseProduct(target_weights_buffer);

	// divide the target weights buffer by the sum to get the new weights
	auto sum = target_weights_buffer.array().abs().sum();
	if (sum > 0.0)
	{
		target_weights_buffer /= sum;
	}
	assert(!target_weights_buffer.array().isNaN().any());
}


//============================================================================
void
Strategy::step() noexcept
{
	// evaluate the strategy with the current market prices and weights
	evaluate(m_impl->m_target_weights_buffer);

	// check if exchange took step
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
		// if no action was taken, propogate asset returns to adjust weights
		lateRebalance(m_impl->m_target_weights_buffer);
	}
	assert(!m_impl->m_target_weights_buffer.array().isNaN().any());
	m_step_call = false;
}


//============================================================================
void
Strategy::setNlv(double nlv_new) noexcept
{
	m_impl->m_tracer.setNLV(nlv_new);
}


//============================================================================
void
Strategy::reset() noexcept
{
	m_impl->m_tracer.reset();
	m_impl->m_target_weights_buffer.setZero();
	m_impl->m_ast->reset();
	m_step_call = false;
}


}