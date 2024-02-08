import sys
import time

atlas_py_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Release"
sys.path.append(atlas_py_path)

import AtlasWrap
from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast import *

exchange_id = "test"
portfolio_id = "test_p"
strategy_id = "test_s"
exchange_path_sp500 = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasTest/scripts/data_sp500.h5"
initial_cash = 100.0

hydra = Hydra()

exchange = hydra.addExchange(exchange_id, exchange_path_sp500)
portfolio = hydra.addPortfolio(portfolio_id, exchange, initial_cash)
hydra.build()

st = time.time()
read_close = AssetReadNode.make("close", 0, exchange)
read_50_ma = AssetReadNode.make("50_ma", 0, exchange)
spread = AssetDifferenceNode(read_close, read_50_ma)
op_variant = AssetOpNodeVariant(spread)

exchange_view = ExchangeViewNode(exchange, op_variant)
exchange_view.setFilter(ExchangeViewFilterType.GREATER_THAN, 0.0)
allocation = AllocationNode(exchange_view)
strategy_node = StrategyNode(allocation, portfolio)
strategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, 1.0))

hydra.run()
et = time.time()

print(f"Time: {(et - st)} s")
print(f"Time: {(et - st) * 1000000.0} us")

ret = (strategy.getNLV() - initial_cash) / initial_cash
print(f"Epsilon: {ret - 2.6207}")