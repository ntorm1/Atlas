import unittest
import os

import context

import AtlasWrap

TEST_FILE_DIR = "files"
TEST_FILE_1 = "test_strategy_1.toml"
PORTFOLIO_ID = "test_portfolio_1"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"
EXCHANGE_CSV = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchange1"


class TestParser(unittest.TestCase):
    def setUp(self) -> None:
        self.hydra = AtlasWrap.Hydra()
        self.exchange = self.hydra.addExchange(EXCHANGE_ID, EXCHANGE_CSV,  "%Y-%m-%d")
        self.portfolio = self.hydra.addPortfolio(PORTFOLIO_ID, self.exchange, 100.0)

    def get_full_path(self, file_name):
        return os.path.join(os.path.dirname(__file__),TEST_FILE_DIR, file_name)

    def test_parse(self):
        parser = AtlasWrap.Parser(self.hydra)
        parser.parse(self.get_full_path(TEST_FILE_1))
        self.assertTrue(True)
    
if __name__ == '__main__':
    unittest.main()