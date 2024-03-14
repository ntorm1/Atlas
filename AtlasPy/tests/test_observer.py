import unittest
import os

import pandas as pd
import numpy as np
import statsmodels.api as sm


import context
from AtlasWrap import *
from AtlasWrap import AtlasPy
from AtlasPy.model import *

HYDRA_DIR = "files/hydra1"
TEST_FILE_1 = "test_strategy_1.toml"
PORTFOLIO_ID = "test_portfolio_1"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"

EXCHANGE_CSV = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchangeVBT"


class TestParser(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__),HYDRA_DIR)
        parser = Parser(hydra_path)
        self.hydra = parser.getHydra()
        self.exchange = self.hydra.getExchange(EXCHANGE_ID)
        self.portfolio = self.hydra.getPortfolio(PORTFOLIO_ID)

    def get_df(self):
        ticker = "BTC-USD"
        path = os.path.join(EXCHANGE_CSV,f"{ticker}.csv")  
        df = pd.read_csv(path)
        df["Date"] = pd.to_datetime(df["Date"]) 
        df = df.set_index("Date")
        return df

    def test_parse(self):
        window = 5
        self.hydra.getExchange(EXCHANGE_ID)
        self.hydra.getPortfolio(PORTFOLIO_ID)
        window = 5
        close =AssetReadNode.make("Close", 0, self.exchange)
        prev_close =AssetReadNode.make("Close", -1, self.exchange)
        change =AssetOpNode.make(
            close,
            prev_close,
           AssetOpType.SUBTRACT
        )
        close_arg_max = self.exchange.registerObserver(TsArgMaxObserverNode("arg_max", change, window))
        close_max = self.exchange.registerObserver(MaxObserverNode("max", change, window))
        self.exchange.enableNodeCache("close_arg_max",close_arg_max, False)
        self.exchange.enableNodeCache("close_max",close_max, False)

        ev =ExchangeViewNode.make(self.exchange, close)
        allocation =AllocationNode.make(
            ev
        )
        strategy_node_signal = StrategyNode.make(allocation, self.portfolio)
        _ = self.hydra.addStrategy(Strategy(STRATEGY_ID, strategy_node_signal, 1.0), True)
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
        strategy = self.hydra.addStrategy(Strategy(STRATEGY_ID, strategy_node_signal, 1.0), True)

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
        strategy = self.hydra.addStrategy(Strategy(STRATEGY_ID, strategy_node_signal, 1.0), True)
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
        strategy = self.hydra.addStrategy(Strategy(STRATEGY_ID, strategy_node_signal, 1.0), True)
        self.hydra.run()

        df = self.get_df()
        btc_idx = self.exchange.getAssetIndex("BTC-USD")
        df["close_open_cov_atlas"] = cov.cache()[btc_idx].T
        df["close_open_cov_pd"] = df["Close"].rolling(window).cov(df["Open"])
        df = df[['close_open_cov_atlas', 'close_open_cov_pd']]
        df.replace(np.nan, 0, inplace=True)
        self.assertTrue(np.allclose(df["close_open_cov_atlas"], df["close_open_cov_pd"]))

    def test_lr(self):
        walk_forward_window = 3
        training_window = 5
        close = AssetReadNode.make("Close", 0, self.exchange)
        open = AssetReadNode.make("Open", 0, self.exchange)
        prev_close = AssetReadNode.make("Close", -1, self.exchange)

        feat1 = AssetOpNode.make(
            close,
            prev_close,
            AssetOpType.SUBTRACT
        )
        feat2 = AssetOpNode.make(
            close,
            open,
            AssetOpType.SUBTRACT
        )
        target = ModelTarget(
            close,
            ModelTargetType.ABSOLUTE,
            2
        )
        config = ModelConfig(
            training_window = training_window,
            walk_forward_window = walk_forward_window,
            model_type = ModelType.LINEAR_REGRESSION,
            exchange = self.exchange,
        )
        lr_config = LinearRegressionModelConfig(
            config,
            LinearRegressionSolver.LDLT
        )
        lr_config.fit_intercept = True
        lr_model = LinearRegressionModel(
            "lr_model",
            [feat1, feat2],
            target,
            lr_config
        )
        self.exchange.registerModel(lr_model)

        for i in range(training_window-1):
            self.hydra.step()

        x = lr_model.getX()
        y = lr_model.getY()
        df = self.get_df()
        df["feat_1"] = df["Close"] - df["Close"].shift(1)
        df["feat_2"] = df["Close"] - df["Open"]
        df["target"] = df["Close"].shift(-2)
        self.assertAlmostEqual(y[0], df["target"].iloc[1])
        self.assertAlmostEqual(x[0,0], df["feat_1"].iloc[1])
        self.assertAlmostEqual(x[0,1], df["feat_2"].iloc[1])
        self.assertAlmostEqual(x[1,0], df["feat_1"].iloc[2])
        self.assertAlmostEqual(x[1,1], df["feat_2"].iloc[2])

        self.hydra.step()
        self.hydra.step()

        x = lr_model.getX()
        y = lr_model.getY()
        model = sm.OLS(y[:-2], x[:-2,:]).fit()
        params = np.array(model.params)
        self.assertTrue(np.allclose(params, lr_model.getTheta()))

        self.hydra.step()
        x = lr_model.getX()
        y = lr_model.getY()
        x_pandas = df[["feat_1", "feat_2"]].iloc[2:5].values
        y_pandas = df["target"].iloc[2:5].values
        self.assertTrue(np.allclose(x_pandas, x[1:4,:-1]))
        self.assertTrue(np.allclose(y_pandas, y[1:4]))
        model = sm.OLS(y[1:4], x[1:4,:]).fit()
        params = np.array(model.params)
        self.assertTrue(np.allclose(params, lr_model.getTheta()))

        for i in range(walk_forward_window):
            self.hydra.step()
        x = lr_model.getX()
        y = lr_model.getY()
        x_pandas = df[["feat_2", "feat_1"]].iloc[5:8].values
        x_pandas = sm.add_constant(x_pandas)
        x_pandas = x_pandas[:, [2, 1, 0]]
        y_pandas = df["target"].iloc[5:8].values
        model = sm.OLS(y_pandas, x_pandas).fit()
        params = np.array(model.params)
        self.assertTrue(np.allclose(params, lr_model.getTheta()))
    
if __name__ == '__main__':
    unittest.main()