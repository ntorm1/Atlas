import unittest
import logging
import os
import sys

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import AtlasPy
from AtlasPy import *
from AtlasPy import atlas_internal
from AtlasPy.strategy import ImmediateStrategy

HYDRA_DIR = "files/hydra1"
HYDRA_DIR_2 = "files/hydra2"
HYDRA_DIR_MULTI = "files/hydraMulti"
HYDRA_DIR_SP500 = "files/hydra_sp500"
TEST_FILE_1 = "test_strategy_1.toml"
PORTFOLIO_ID = "test_portfolio_1"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"


EXCHANGE_PATH = (
    r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/tests/files/exchangeVBT"
)
