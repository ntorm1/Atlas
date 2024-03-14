import unittest
import os

import pandas as pd
import numpy as np

import context
import AtlasWrap

HYDRA_DIR = "files/hydra1"
TEST_FILE_1 = "test_strategy_1.toml"
PORTFOLIO_ID = "test_portfolio_1"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"

EXCHANGE_CSV = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchangeVBT"


class TestParser(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__),HYDRA_DIR)
        parser = AtlasWrap.Parser(hydra_path)
        self.hydra = parser.getHydra()
        self.exchange = self.hydra.getExchange(EXCHANGE_ID)

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
        self.hydra.run()

        df = self.get_df()
        btc_idx = self.exchange.getAssetIndex("BTC-USD")
        close_arg_max = self.exchange.getObserver("close_arg_max")
        close_max = self.exchange.getObserver("close_max")
        assert close_arg_max is not None
        assert close_max is not None
        df["close_arg_max_atlas"] = close_arg_max.cache()[btc_idx].T
        df["close_max_atlas"] = close_max.cache()[btc_idx].T
        df["close_max_pd"] = df["Close"].diff().rolling(window).max()
        df["close_arg_max_pd"] = df["Close"].diff().rolling(window).apply(np.argmax).add(1)
        df = df.iloc[window:]
        self.assertTrue(np.allclose(df["close_max_atlas"], df["close_max_pd"]))
        self.assertTrue(np.allclose(df["close_arg_max_atlas"], df["close_arg_max_pd"]))
    
if __name__ == '__main__':
    unittest.main()