from AtlasPy.core import Strategy
from AtlasPy.ast import *
import numpy as np

strategy_id = "test_strategy"
exchange_id = "test"
portfolio_id = "test_portfolio"
alloc = 1.000000

def build(hydra):
	hydra.removeStrategy(strategy_id)
	exchange = hydra.getExchange(exchange_id)
	cov_trigger = PeriodicTriggerNode.make(exchange, 1)
	trigger_node = PeriodicTriggerNode.make(exchange, 5)
	
	covariance_node = exchange.getCovarianceNode(
		"30_PERIOD_COV",cov_trigger, 30, CovarianceType.FULL
	)
	portfolio = hydra.getPortfolio(portfolio_id)
	
	# get 5 period return for each asset
	asset_read_node = AssetReadNode.make("close", 0, exchange)
	asset_read_node_prev = AssetReadNode.make("close", -5, exchange)
	spread = AssetOpNode.make(asset_read_node, asset_read_node_prev, AssetOpType.DIVIDE)
	exchange_view = ExchangeViewNode.make(exchange, spread)

	
	# rank assets by the returns, flag the bottom 2 and top 2
	rank_node = EVRankNode.make(
		exchange_view,
		EVRankType.NEXTREME,
		2
	)

	# short the bottom 2 assets and go long the top 2
	allocation = AllocationNode.make(
		exchange_view,
		AllocationType.CONDITIONAL_SPLIT,
		0.0,
		0.025
	)
	
	allocation.setTradeLimit(TradeLimitType.STOP_LOSS, .05)
	trade_limit = allocation.getTradeLimitNode()
	TradeLimitNode.setTakeProfit(trade_limit, .05)
	
	take_profit_grid_dim = GridDimension.make(
		name="Take Profit",
		dimension_values=list(np.linspace(0.01, .1, 50)),
		node=trade_limit,
		getter=trade_limit.takeProfitGetter(),
		setter=trade_limit.takeProfitSetter()
	)
	stop_loss_grid_dim = GridDimension.make(
		name="Stop Loss",
		dimension_values=list(np.linspace(0.01, .1, 50)),
		node=trade_limit,
		getter=trade_limit.stopLossGetter(),
		setter=trade_limit.stopLossSetter()
	)
	
	inv_vol_weight = InvVolWeight(covariance_node, .1)
	allocation.setWeightScale(inv_vol_weight)
	strategy_node = StrategyNode.make(allocation, portfolio)
	strategy_node.setTrigger(trigger_node)
	strategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, alloc), True)
	strategy.setGridDimmensions((take_profit_grid_dim, stop_loss_grid_dim))
	strategy.setVolTracer(covariance_node)
	strategy.enableTracerHistory(TracerType.NLV)
	strategy.enableTracerHistory(TracerType.VOLATILITY)