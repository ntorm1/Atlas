import unittest
import os

import context

import AtlasWrap

HYDRA_DIR = "files/hydra1"
TEST_FILE_1 = "test_strategy_1.toml"
PORTFOLIO_ID = "test_portfolio_1"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"

EXCHANGE_CSV = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchange1"


class TestParser(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__),HYDRA_DIR)
        parser = AtlasWrap.Parser(hydra_path)
        self.hydra = parser.getHydra()

        #self.exchange = self.hydra.addExchange(EXCHANGE_ID, EXCHANGE_CSV,  "%Y-%m-%d")
        #self.portfolio = self.hydra.addPortfolio(PORTFOLIO_ID, self.exchange, 100.0)

    def test_parse(self):
        self.hydra.getExchange(EXCHANGE_ID)
        self.hydra.getPortfolio(PORTFOLIO_ID)
        self.assertTrue(True)
    
if __name__ == '__main__':
    unittest.main()