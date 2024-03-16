import unittest
import logging
import os

import pandas as pd
import numpy as np

import context
from AtlasWrap import *
from AtlasWrap import AtlasPy

HYDRA_DIR = "files/hydra1"
TEST_FILE_1 = "test_strategy_1.toml"
PORTFOLIO_ID = "test_portfolio_1"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"

EXCHANGE_CSV = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchangeVBT"

logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)




class TestStrategy(unittest.TestCase):
    def setUp(self) -> None:
        logging.info("Setting up TestStrategy test")
        hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR)
        parser = Parser(hydra_path)
        self.hydra = parser.getHydra()
        self.exchange = self.hydra.getExchange(EXCHANGE_ID)
        self.portfolio = self.hydra.getPortfolio(PORTFOLIO_ID)
        logging.info("TestStrategy Setup complete")

    def test_strategy_base(self):
        pass


if __name__ == "__main__":
    unittest.main()
