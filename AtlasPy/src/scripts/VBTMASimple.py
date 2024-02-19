import time
import vectorbt as vbt

price = vbt.YFData.download('BTC-USD', start='2018-01-01', end = '2024-01-01')

st = time.time()
close = price.get('Close')
fast_ma = vbt.MA.run(close, 5, short_name='fast_ma')
slow_ma = vbt.MA.run(close, 10, short_name='slow_ma')
entries = fast_ma.ma_crossed_above(slow_ma)
exits = fast_ma.ma_crossed_below(slow_ma)
pf = vbt.Portfolio.from_signals(close, entries, exits, fees=0.000)
et = time.time()

"""
Elapsed time: 38.49 ms
Total return: 2.09876466123077
"""

print(f'Elapsed time: {1000*(et-st):.2f} ms')
print(f'Total return: {pf.total_return()}')