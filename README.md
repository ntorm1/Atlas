# Atlas
![alt text](https://github.com/ntorm1/Atlas/blob/main/util/AtlasX.png)
## About
Atlas is a high performance Algorthmic Trading Backtesting library written in C++23 with a Python wrapper build using pybind11 and a GUI based on QT. It is split into five parts

1. Atlas (core c++ runtime build is Visual Studio DLL Project)
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

To build
- cd to AtlasX/external/QScintilla/src. Open x64 Native Tools Command Prompt
- run qmake
- run nmake /f Makefile.Debug (and release), then copy dll files to x64/debug and release
- Open AtlasX.sln and build.


## Getting Started
Currently the only support data form is .h5 files and csv files. Here is a dummy example of what the format should look like. Provided you have a dataframe df that looks like this

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


You can also build dataset using a folder containing onlny csv files. They are expected to all have datetime index in the first column with the same format.
For example use VectorBT + yfinance to download btc data
```python
price = vbt.YFData.download('BTC-USD', start='2018-01-01', end = '2024-01-01')
df= price.data['BTC-USD']["Close"]
df.to_csv(out_csv_path_)
```



Using this data we can use VectorBT exaomple from their documentation to build moving average
crossover strategy for a baseline comparison. 

```python
st = time.time()
close = price.get('Close')
fast_ma = vbt.MA.run(close, 50, short_name='fast_ma')
slow_ma = vbt.MA.run(close, 200, short_name='slow_ma')
entries = fast_ma.ma_crossed_above(slow_ma)
exits = fast_ma.ma_crossed_below(slow_ma)
pf = vbt.Portfolio.from_signals(close, entries, exits, fees=0.000)
et = time.time()
print(f'Elapsed time: {1000*(et-st):.2f} ms')
print(f'Total return: {pf.total_return()}')

# Elapsed time: 48.20 ms
# Total return: 4.851262581813536
```

Now we can use AtlasPy to build the same strategy and compare the results. First we need to import the library and define the runtime.

```python
import sys
atlas_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Release"
sys.path.append(atlas_path)

from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast import *
```

Define the runtime

```python
exchange_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/src/exchangeVBT"
strategy_id = "test_strategy"
exchange_id = "test_exchange"
portfolio_id = "test_portfolio"

hydra = Hydra()
intial_cash = 100.0
exchange = hydra.addExchange(exchange_id, exchange_path, "%Y-%m-%d %H:%M:%S")
portfolio = hydra.addPortfolio(portfolio_id, exchange, intial_cash)
hydra.build()
```

Define the strategy. The below strategy will go long all assets if their fast moving average is above their slow, and short vice versa.

```python
st = time.time()
fast_n = 50
slow_n = 200

close = AssetReadNode.make("Close", 0, exchange)
fast_ma = AssetScalerNode(
    exchange.registerObserver(SumObserverNode(close, fast_n)),
    AssetOpType.DIVIDE,
    fast_n
)
slow_ma = AssetScalerNode(
    exchange.registerObserver(SumObserverNode(close, slow_n)),
    AssetOpType.DIVIDE,
    slow_n
)
spread = AssetOpNode.make(fast_ma, slow_ma, AssetOpType.SUBTRACT)
spread_filter = ExchangeViewFilter(ExchangeViewFilterType.GREATER_THAN, 0.0, None)
exchange_view = ExchangeViewNode.make(exchange, spread, spread_filter)

allocation = AllocationNode.make(exchange_view)
strategy_node = StrategyNode.make(allocation, portfolio)
strategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, 1.0), True)
strategy.enableTracerHistory(TracerType.NLV)
hydra.run()

nlv = strategy.getHistory(TracerType.NLV)
returns = nlv[-1] / nlv[0] - 1

et = time.time()
print(f'Elapsed time: {1000*(et-st):.2f} ms')
print(f'Total return: {returns}')

# Elapsed time: 0.72 ms
# Total return: 4.851262581813547
```

See AtlasPy test files for more examples and comparisons