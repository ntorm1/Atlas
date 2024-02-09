import sys
import os
import unittest
import pandas as pd
import numpy as np

atlas_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Debug"
sys.path.append(atlas_path)


from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast import *


exchange_path_sp500_ma = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/test/data_sp500_ma.h5"
exchange_path = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasTest/scripts/data.h5"

class AllocTest(unittest.TestCase):
    
    def setUp(self) -> None:
        self.hydra = Hydra()
        self.intial_cash = 100.0
        self.exchange = self.hydra.addExchange("test", exchange_path)
        self.portfolio = self.hydra.addPortfolio("test_p", self.exchange, self.intial_cash)
        self.strategy_id = "test_s"
        self.asset_id1 = "asset1"
        self.asset_id2 = "asset2"
        self.asset1_index = self.exchange.getAssetIndex(self.asset_id1)
        self.asset2_index = self.exchange.getAssetIndex(self.asset_id2)

        self.asset1_close = [101,103,105,106];
        self.asset2_close = [101.5,99,97,101.5,101.5,96];

    def testBuild(self) -> None:
        timestamps = self.exchange.getTimestamps()
        assert len(timestamps) == 6

    def testSimpleAlloc(self) -> None:
        read_close = AssetReadNode.make("close", 0, self.exchange)
        exchange_view = ExchangeViewNode.make(self.exchange, read_close)
        allocation = AllocationNode.make(
            exchange_view,
            AllocationType.UNIFORM,
            0.0
        )
        
        # build final strategy and insert into hydra
        strategy_node = StrategyNode.make(allocation, self.portfolio)
        self.hydra.build()
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node, 1.0))
        self.hydra.step()
        nlv = strategy.getNLV()
        assert nlv == self.intial_cash
        self.hydra.step()

        asset_2_return = (self.asset2_close[1] - self.asset2_close[0]) / self.asset2_close[0];
        nlv = self.intial_cash * (1 + asset_2_return)
        assert strategy.getNLV() == nlv
        
        self.hydra.step()
        asset_2_return2 = (self.asset2_close[2] - self.asset2_close[1]) / self.asset2_close[1]
        asset_1_return2 = (self.asset1_close[1] - self.asset1_close[0]) / self.asset1_close[0]
        avg_return = (.5 * asset_2_return2) + (.5 * asset_1_return2)
        nlv *= (1.0 + avg_return)
        assert strategy.getNLV() == nlv


    def testFixedAlloc(self) -> None:
        alloc = [(self.asset_id1, 0.3), (self.asset_id2, 0.7)]
        allocation = FixedAllocationNode.make( alloc, self.exchange, 0.0)
        strategy_node = StrategyNode.make(allocation, self.portfolio)

        trigger_node = PeriodicTriggerNode.make(self.exchange, 2)
        strategy_node.setTrigger(trigger_node)
        strategy_node.setWarmupOverride(1)

        self.hydra.build()
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node, 1.0))
        commission_manager = strategy.initCommissionManager()
        fixed_commission = 1.0
        pct_commission = 0.001
        commission_manager.setFixedCommission(fixed_commission)
        commission_manager.setCommissionPct(pct_commission)

        # test warmup override 
        self.hydra.step()
        allocation = strategy.getAllocationBuffer()
        self.assertTrue(np.allclose(allocation, np.zeros_like(allocation)))
        self.assertAlmostEqual(strategy.getNLV(), self.intial_cash)

        # trigger node prevents alloc on second step so repeat first step
        self.hydra.step()
        self.assertTrue(np.allclose(allocation, np.zeros_like(allocation)))
        self.assertAlmostEqual(strategy.getNLV(), self.intial_cash)

        # now we are 30% in asset1 and 70% in asset2. Two trades needed
        self.hydra.step()
        self.assertTrue(np.allclose(allocation, np.array([0.3, 0.7])))
        commission = 2 * fixed_commission + pct_commission * self.intial_cash
        self.assertAlmostEqual(strategy.getNLV(), self.intial_cash - commission)
        self.assertAlmostEqual(allocation.sum(), 1.0)

        # trigger node prevents alloc
        self.hydra.step()
        # get deepy copy of strategy allocation buffer
        weights_2 = np.copy(strategy.getAllocationBuffer())

        # fixed alloc forced rebalance, should have two fixed commissions and two pct commissions
	    # proportional to the size of the rebalance
        nlv = strategy.getNLV()
        self.hydra.step()
        returns = self.exchange.getMarketReturns()
        portfolio_return = np.dot(returns, weights_2)
        nlv = nlv * (1 + portfolio_return)

        weights_delta = np.abs(weights_2 - strategy.getAllocationBuffer())
        commission = 2 * fixed_commission + (pct_commission * nlv * weights_delta).sum()
        self.assertAlmostEqual(allocation.sum(), 1.0)
        self.assertAlmostEqual(strategy.getNLV(), nlv - commission)


class RiskTest(unittest.TestCase):

    def setUp(self) -> None:
        self.data = pd.read_csv("data.csv")
        self.data["Date"] = pd.to_datetime(self.data["Date"])
        self.data.set_index("Date", inplace=True)
        self.hydra = Hydra()
        self.exchange = self.hydra.addExchange("test", exchange_path_sp500_ma)
        self.portfolio = self.hydra.addPortfolio("test_p", self.exchange, 100.0)
        self.strategy_id = "test_s"

    def testInit(self):
        self.assertEqual(self.exchange.getName(),"test")

    # helper function to run to a specific date
    def runTo(self, date):
        timestamps = self.exchange.getTimestamps()
        timestamps = pd.to_datetime(timestamps, unit="ns")
        index = self.data.index.get_loc(date)
        for i in range(index):
            self.hydra.step()

    def testCovariance(self):
        monthly_trigger_node = StrategyMonthlyRunnerNode.make(self.exchange)
        covariance_node = self.exchange.getCovarianceNode("30_PERIOD_COV", monthly_trigger_node, 30)
        
        # data starts 2010-01-04, and the first day of february will not 
        # have 20 days of data to calculate covariance, so the first trigger date
        # will be 2010-03-01
        self.hydra.build()
        self.runTo("2010-03-01")

        cov_matrix = covariance_node.getCovarianceMatrix()
        cov_sum = cov_matrix.sum()
        self.assertAlmostEqual(cov_sum, 0.000000000)
        self.hydra.step()
        cov_matrix = covariance_node.getCovarianceMatrix()

        # get the covariance matrix from pandas, the last row should be 2010-03-01
        # and the first row should be 21 rows before that
        end_idx = self.data.index.get_loc("2010-03-01")
        start_idx = end_idx - 30
        matrix_subset = self.data.iloc[start_idx:end_idx+1,:]
        ordered_columns = sorted(matrix_subset.columns, key=lambda x: self.exchange.getAssetMap()[x])
        matrix_subset = matrix_subset[ordered_columns]
        matrix_subset = matrix_subset.pct_change().dropna().cov(ddof=1)

        # assert all values are within 1e-6
        self.assertTrue(np.allclose(cov_matrix, matrix_subset, atol=1e-8))

    def testRankNode(self):
        monthly_trigger_node = StrategyMonthlyRunnerNode.make(self.exchange)
        
        asset_read_node = AssetReadNode.make("close", 0, self.exchange)
        asser_read_previouse_node = AssetReadNode.make("close", -1, self.exchange)
        spread = AssetOpNode.make(asset_read_node, asser_read_previouse_node, AssetOpType.DIVIDE)
        exchange_view = ExchangeViewNode.make(self.exchange, spread)

        # rank assets by 1 period return, flag the bottom 2 and top 2
        rank_node = EVRankNode.make(
            exchange_view,
            EVRankType.NEXTREME,
            2
        )
        
        # short the bottom 2 assets, long the top 2
        allocation = AllocationNode.make(
            rank_node,
            AllocationType.CONDITIONAL_SPLIT,
            0.0
        )
        
        # build final strategy and insert into hydra
        strategy_node = StrategyNode.make(allocation, self.portfolio)
        strategy_node.setTrigger(monthly_trigger_node)
        self.hydra.build()
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node, 1.0))
        
        self.runTo("2010-02-01")

        allocation = strategy.getAllocationBuffer()
        self.assertTrue(np.allclose(allocation, np.zeros_like(allocation)))

        self.hydra.step()
        current_time = self.exchange.getCurrentTimestamp()
        current_time = pd.to_datetime(current_time, unit="ns")
        # get data from current time including the previous day
        data = self.data.loc[:current_time]
        returns = data.pct_change().iloc[-1]
        ordered_columns = sorted(data.columns, key=lambda x: self.exchange.getAssetMap()[x])
        returns = returns[ordered_columns]
        # flip the two largest to 1 and the two smallest to -1, rest to 0
        largest_indices = returns.nlargest(2).index
        smallest_indices = returns.nsmallest(2).index
        returns.loc[largest_indices] = 1
        returns.loc[smallest_indices] = -1
        returns.loc[~returns.index.isin(largest_indices.union(smallest_indices))] = 0
        returns = returns.values
        returns /= np.abs(returns).sum()
        allocation = strategy.getAllocationBuffer()
        self.assertTrue(np.allclose(allocation, returns))


if __name__ == "__main__":
    unittest.main()