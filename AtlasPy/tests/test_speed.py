import pandas as pd
import numpy as np
import time

from context import *

hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR_SP500)

counts = [10, 20, 50, 75, 100, 200, 300, 500, 700, 1000, 2000]
cps = []
for strategy_count in counts:
    parser = Parser(hydra_path)
    hydra = parser.getHydra()
    exchange = hydra.getExchange(EXCHANGE_ID)
    intial_cash = 100.0
    root_strategy = MetaStrategy("root", exchange, None, intial_cash)
    hydra.addStrategy(root_strategy, True)

    for i in range(strategy_count):
        asset_read_node = AssetReadNode.make("close", 0, exchange)
        asser_read_previouse_node = AssetReadNode.make("close", -1, exchange)
        spread = AssetOpNode.make(
            asset_read_node, asser_read_previouse_node, AssetOpType.DIVIDE
        )
        spread_filter = ExchangeViewFilter(ExchangeViewFilterType.GREATER_THAN, 1, None)
        exchange_view = ExchangeViewNode.make(exchange, asset_read_node)
        allocation = AllocationNode.make(exchange_view)
        strategy_node = StrategyNode.make(allocation)
        strategy = ImmediateStrategy(
            exchange,
            root_strategy,
            STRATEGY_ID + f"_{i}",
            1.0,
            strategy_node,
        )
        _ = root_strategy.addStrategy(strategy, True)

    st = time.time()
    hydra.run()
    et = time.time()

    row_count = len(exchange.getTimestamps())
    asset_count = len(exchange.getAssetMap().keys())
    candles = row_count * asset_count * strategy_count

    elapsed_time_formatted = "{:,.2f}".format(et - st)
    candles_formatted = "{:,.0f}".format(candles)
    candles_per_second_formatted = "{:,.2f}".format(candles / (et - st))
    cps.append(candles / (et - st))

df = pd.DataFrame({"strategy_count": counts, "candles_per_second": cps})
df.to_csv("candles_per_second.csv", index=False)
print(df)
