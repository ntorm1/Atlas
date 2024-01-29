# Atlas
![alt text](https://github.com/ntorm1/Atlas/blob/main/util/AtlasX.png)
## About
Atlas is a high performance Algorthmic Trading Backtesting library written in C++23 with a Python wrapper build using pybind11 and a GUI based on QT. It is split into five parts

1. Atlas (core c++ runtime build is Visual Studio Project)
2. AtlasTest (c++ test suite)
3. AtlasPy (pybind11 wrapper)
3. AtlasPerf (dummy c++ project)
4. AtlasX (GUI based on Qt)


## Building
Install requires visual studio and vcpkg manager and the following packages installed:
1. Eigen
2. H5CPP
3. Pybind11 (AtlasPy)
4. Qt (AtlasX)
To build simply open the Visual Studio Solution and build AtlasPy. The output of which is a .pyd extension that can be directly imported into Python.

## Getting Started
Currently the only support data form is .h5 files. Here is a dummy example of what the format should look like. Provided you have a dataframe df that looks like this

| Date                | Close         | ma_fast       | ma_slow       |
|---------------------|---------------|---------------|---------------|
| 1547942400000000000 | 3601.013672   | 3655.933936   | 3786.643591   |
| 1548028800000000000 | 3576.032471   | 3644.800635   | 3773.269214   |
| 1548115200000000000 | 3604.577148   | 3639.128247   | 3756.327600   |
| 1548201600000000000 | 3585.123047   | 3642.345239   | 3743.746692   |
| 1548288000000000000 | 3600.865479   | 3631.826562   | 3730.904089   |
| ...                 | ...           | ...           | ...           |

Then you can save it using the following format. Note that you must have a single "Close" column (not cast sensitive) use to determine prices. Also, you can have arbitrary number of tickers, as long as they all have the same frequency.

```python
output_path = os.path.join(os.getcwd(), 'data_fast.h5')
if os.path.exists(output_path):
    os.remove(output_path)

ticker = "BTCUSD"
with h5py.File(output_path, "a") as file:
        stock_data = df.to_numpy()
        index = df.index.map(lambda timestamp: int(timestamp)).to_numpy()
        dataset = file.create_dataset(f"{ticker}/data", data=stock_data)
        file.create_dataset(
                f"{ticker}/datetime",
                data = index,
        )
        for column in df.columns:
            dataset.attrs[column] = column
```


Now to build the strategy, start by importing the core componenets

```python
import sys
atlas_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Release"
sys.path.append(atlas_path)

from AtlasPy import TracerType
from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast *
```

Define the runtime

```python
exchange_id = "test"
portfolio_id = "test_p"
strategy_id = "test_s"
exchange_path_fast = "C:/Users/ ... /data_fast.h5"
initial_cash = 100.0
hydra = Hydra()
exchange = hydra.addExchange(exchange_id, exchange_path_fast)
portfolio = hydra.addPortfolio(portfolio_id, exchange, initial_cash)
hydra.build()
```

Define the strategy. The below strategy will go long all assets if their fast moving average is above their slow, and short vice versa.

```python
read_fast = AssetReadNodeWrapper.make("ma_fast", 0, exchange)
read_slow = AssetReadNodeWrapper.make("ma_slow", 0, exchange)
spread = AssetDifferenceNodeWrapper.make(read_fast, read_slow)
op_variant = AssetOpNodeVariant.make(spread)

filter = ExchangeViewFilter(ExchangeViewFilterType.GREATER_THAN, 0.0)
exchange_view = ExchangeViewNode(exchange, op_variant, filter)
allocation = AllocationNodeWrapper.make(exchange_view)
strategy_node = StrategyNodeWrapper.make(allocation, portfolio)
strategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, alloc), True)
strategy.enableTracerHistory(TracerType.WEIGHTS)
strategy.enableTracerHistory(TracerType.NLV)
```

Now we can execute and get the total return, we compare our results to VectorBT to ensure accuracy here. 

```python
time_sum = 0
n = 1
for i in range(n):
    st = time.perf_counter_ns()
    hydra.run()
    et = time.perf_counter_ns()
    time_sum += et - st

avg_time_micros = (time_sum / n) / 1000

tr = (strategy.getNLV() - initial_cash) / initial_cash
print(f"Time elapsed Avg: {avg_time_micros:.3f} us")
print(f"Total return: {tr:.3%}")
print(f"Epsilon: {tr - ret_fast}")
"""
Time elapsed Avg: 32.400 us
Total return: 63.519%
Epsilon: 1.7763568394002505e-15
""
```

See AtlasPy test files for more examples and comparisons