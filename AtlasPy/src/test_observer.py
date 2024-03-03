import sys
import os
import time
import unittest
import pandas as pd
import numpy as np

# show pandas columns with 3 points of precision
pd.set_option('display.float_format', lambda x: '%.3f' % x)

atlas_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Debug"
sys.path.append(atlas_path)


from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast import *


exchange_path_sp500_ma = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/test/data_sp500_ma.h5"
exchange_path = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasTest/scripts/data.h5"
exchange_csv = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchange1"



class SimpleObserverTest(unittest.TestCase):
    def setUp(self) -> None:
        self.exchange_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchangeVBT"
        self.hydra = Hydra()
        self.exchange = self.hydra.addExchange("test", self.exchange_path,  "%Y-%m-%d %H:%M:%S")
        self.portfolio = self.hydra.addPortfolio("test_p", self.exchange, 100.0)
        self.strategy_id = "test_s" 
        self.hydra.build()


    def get_df(self):
        ticker = "BTC-USD"
        path = os.path.join(self.exchange_path,f"{ticker}.csv")  
        df = pd.read_csv(path)
        df["Date"] = pd.to_datetime(df["Date"]) 
        df = df.set_index("Date")
        return df

    def test_max_observer(self):
        window = 5
        close = AssetReadNode.make("Close", 0, self.exchange)
        prev_close = AssetReadNode.make("Close", -1, self.exchange)
        change = AssetOpNode.make(
            close,
            prev_close,
            AssetOpType.SUBTRACT
        )
        close_arg_max = self.exchange.registerObserver(TsArgMaxObserverNode("arg_max", change, window))
        close_max = self.exchange.registerObserver(MaxObserverNode("max", change, window))
        self.exchange.enableNodeCache("close_arg_max",close_arg_max, False)
        self.exchange.enableNodeCache("close_max",close_max, False)

        ev = ExchangeViewNode.make(self.exchange, close)
        allocation = AllocationNode.make(
            ev
        )
        strategy_node_signal = StrategyNode.make(allocation, self.portfolio)
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node_signal, 1.0), True)
        self.hydra.run()

        df = self.get_df()
        btc_idx = self.exchange.getAssetIndex("BTC-USD")
        df["close_arg_max_atlas"] = close_arg_max.cache()[btc_idx].T
        df["close_max_atlas"] = close_max.cache()[btc_idx].T
        df["close_max_pd"] = df["Close"].diff().rolling(window).max()
        df["close_arg_max_pd"] = df["Close"].diff().rolling(window).apply(np.argmax).add(1)
        df = df.iloc[window:]
        self.assertTrue(np.allclose(df["close_max_atlas"], df["close_max_pd"]))
        self.assertTrue(np.allclose(df["close_arg_max_atlas"], df["close_arg_max_pd"]))

    def test_sum_observer(self):
        window = 2
        close = AssetReadNode.make("Close", 0, self.exchange)
        sum_node = self.exchange.registerObserver(SumObserverNode("sum", close, window))
        self.exchange.enableNodeCache("sum",sum_node, False)
        ev = ExchangeViewNode.make(self.exchange, sum_node)
        allocation = AllocationNode.make(ev)
        strategy_node_signal = StrategyNode.make(allocation, self.portfolio)
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node_signal, 1.0), True)

        self.hydra.run()
        df = self.get_df()
        btc_idx = self.exchange.getAssetIndex("BTC-USD")
        df["close_sum_atlas"] = sum_node.cache()[btc_idx].T
        df["close_sum_atlas_diff"] = df["close_sum_atlas"].diff()
        df["close_sum_pd"] = df["Close"].rolling(window).sum()
        df.replace(np.nan, 0, inplace=True)
        self.assertTrue(np.allclose(df["close_sum_atlas"], df["close_sum_pd"]))

    def test_var_observer(self):
        window = 3
        close = AssetReadNode.make("Close", 0, self.exchange)
        prev_close = AssetReadNode.make("Close", -1, self.exchange)
        change = AssetOpNode.make(
            close,
            prev_close,
            AssetOpType.SUBTRACT
        )
        change_squared = AssetFunctionNode(
            change,
            AssetFunctionType.POWER,
            2.0
        )
        change_squared_sum = self.exchange.registerObserver(SumObserverNode("change_squared_sum", change_squared, window))
        self.exchange.enableNodeCache("change_squared_sum",change_squared, False)
        self.exchange.enableNodeCache("change_squared_sum",change_squared_sum, False)
        ev = ExchangeViewNode.make(self.exchange, change_squared_sum)
        allocation = AllocationNode.make(ev)
        strategy_node_signal = StrategyNode.make(allocation, self.portfolio)
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node_signal, 1.0), True)
        self.hydra.run()

        df = self.get_df()
        btc_idx = self.exchange.getAssetIndex("BTC-USD")
        df["close_change_squared_atlas_sum"] = change_squared_sum.cache()[btc_idx].T
        df["close_change_squared_pd_sum"] = df["Close"].diff().pow(2).rolling(window).sum()
        df["close_change_squared_atlas"] = change_squared.cache()[btc_idx].T
        df["close_change_squared_pd"] = df["Close"].diff().pow(2)
        df = df[['close_change_squared_atlas_sum', 'close_change_squared_pd_sum', 'close_change_squared_atlas', 'close_change_squared_pd']]
        df.replace(np.nan, 0, inplace=True)
        self.assertTrue(np.allclose(df["close_change_squared_atlas"], df["close_change_squared_pd"]))

    def test_cov_observer(self):
        window = 5
        close = AssetReadNode.make("Close", 0, self.exchange)
        open_node = AssetReadNode.make("Open", 0, self.exchange)
        cov = self.exchange.registerObserver(CovarianceObserverNode("cov", close, open_node, window))
        self.exchange.enableNodeCache("cov",cov, False)
        ev = ExchangeViewNode.make(self.exchange, cov)
        allocation = AllocationNode.make(ev)
        strategy_node_signal = StrategyNode.make(allocation, self.portfolio)
        strategy = self.hydra.addStrategy(Strategy(self.strategy_id, strategy_node_signal, 1.0), True)
        self.hydra.run()

        df = self.get_df()
        btc_idx = self.exchange.getAssetIndex("BTC-USD")
        df["close_open_cov_atlas"] = cov.cache()[btc_idx].T
        df["close_open_cov_pd"] = df["Close"].rolling(window).cov(df["Open"])
        df = df[['close_open_cov_atlas', 'close_open_cov_pd']]
        df.replace(np.nan, 0, inplace=True)
        self.assertTrue(np.allclose(df["close_open_cov_atlas"], df["close_open_cov_pd"]))



if __name__ == "__main__":
    unittest.main()