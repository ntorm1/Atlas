---
title: Building the Fastest Backtester in Open Source
description: How to backtest a moving average cross over strategy in 15 microseconds.
date: '2024-1-20'
categories:
  - Atlas
published: true
---

#### How to backtest a moving average cross over strategy in 15 microseconds 

## Table of Contents


## Project

Long story short: I have been working on designing and building a high performance backtesting library for Algorithmic trading strategies, and after several complete restarts thought I would make a blog out of the process. The goal of the project is to build a backtesting engine that is made of modular, reusable components that can be use to create complex strategies at incredible speeds. Additionally, the framework should be flexible enough to express these strategies in a fast prototyping language like Python.

As I mentioned above, this is probably the 3rd or 4th complete rewrite of the project. The early evolutions mainly revolved around trying to figure out a design of the overall system only to get far enough along to realize the design was far from ideal and chose to restart. I even tried implementing the backend in Rust, but found their lack of tooling and robust/well documented packages an extreme headache to say the least.

These early prototypes largely revolved around object oriented programming and revolved around complex self referencing data structures and ultimately was much slower than I hoped due to dynamic allocations and horrible cache coherence. For a basic moving average strategy crossover strategy over the SP500, I could get around 3 million candles per second, a huge speed up over existing python event driven frameworks, but not satisfactory. The new and improved engine takes a vectorized approach and has hit speeds of around 150 million candles per second on a single thread for simple strategies. For now I will skip explaining the intial prototypes out of an interest of time, but the entire source code can be found at: https://github.com/ntorm1/AgisCore2-/tree/main

## Atlas
Atlas (https://github.com/ntorm1/Atlas) is a algorithmic trading backtesting framework written in C++ and wrapped in python using pybind11. The easiest way to build it is using Visual Studio and the provided visual studio solution. This includes Atlas (main c++ runtime), AtlasTest (gtest test suite), AtlasPy (pybind11 python wrapper), and AtlasPerf (dummy project). It requires C++23, i.e. std latest in MSVC, and uses vcpkg to manage packages (eigen, h5cpp, pybind11). There is also a cmake project available that uses clang, but I couldn't figure out how to get modules (which I use extensivly) to work in CMAKE and Clang so it just uses plain .hpp headers instead, I will most likely not keep this in sync with the MSVC version. Bottom line, this is about as least portable of a project as you can make, so do with that what you will.

Disclaimer: Atlas is a personal project and is meant as a learning exercise. It has a comically small test suite and is not production ready, mostly likely has bugs. To implement actual strategies in a live environment use a different library. 

With that out of the way, we can take a look at the project at a high level and look at some of the interesting data strucutres and methods used to build a high performance backtester. Atlas is made up of several different components, and to start off I look at the Exchange componenet which is responsible for holding the actual data of the underlying tradable assets.

## Exchange

```cpp
export struct ExchangeImpl
{
	HashMap<String, size_t> asset_id_map;
	HashMap<String, size_t> headers;
	Vector<Asset> assets;
	Vector<Int64> timestamps;
	Vector<Strategy*> registered_strategies;
	Int64 current_timestamp = 0;
	Eigen::MatrixXd data;
	Eigen::MatrixXd returns;
	size_t col_count = 0;
	size_t close_index = 0;
	size_t current_index = 0;

	ExchangeImpl() noexcept
	{
		data = Eigen::MatrixXd::Zero(0, 0);
	}
};
```

The exchange class holds the underlying data of a series of assets that share some commonality, i.e. perhaps they are the entire SP500. The data is loaded in via .h5 files for performance reasons, but I can find my old code for csvs and parquet and add that back in. All assets loaded into any given exchange must have the same set of headers in the same order, and must define a closing price column.

The timestamps vector contains the sorted union of each datetime index of the indivual assets. Additionally, each asset must be a continous subset of it's parent exchange's timestamp vector to make for easier cross-sectional access. The data is stored in a dynamically sized Eigen matrix of doubles where the matrix has rows = #assets, cols = #timestamps * #headers.

The columns are sorted ascending order with respect to time. So if an exchange has two columns: "Close", and "50_MOVING_AVERAGE", the 0th column will be the close prices of each asset at time t0, the 1st column will be the 50_MOVING_AVERAGE at time t0, the 2nd column will be close prices of each asset at time t1, etc..

## Eigen

Before diving into the core strategy component, a word on Eigen. Eigen is used extensivly in the library, all strategies consist of vector operations, and each strategy's core function is to populate an Eigen vector of target allocation weights. I have found Eigen to be extremly well designed and plenty of resources to help out, thus I define a series of types to help define the Atlas runtime.

```cpp
namespace LinAlg
{
export template <typename T>
	using EigenConstColView = Eigen::Block<
        Eigen::Matrix<T, -1, -1, 0, -1, -1>, -1, 1, true>;

export using EigenCwiseProductOp = 
    Eigen::internal::scalar_product_op<double, double>;

...

export template <typename EigenCwiseBinOpType>
using EigenCwiseBinOp =
	const Eigen::CwiseBinaryOp<
    EigenCwiseBinOpType,
    const EigenConstColView<double>, 
    const EigenConstColView<double>
>;
}
```

## Abstract Strategy Tree

To build strategies we borrow on a concept from compiler design, namely the abstract syntax tree. A strategy is made up of several components and consistents of a single eval function that executes in logic in recursive descent style. For now there are only a handle of operation allowed, but hopefully the design lends itself to more complex strategies. Try and ignore the abuse of shared pointers, most should be unique but using shared pointers allows for easy interop with pybind11 and python without having to make a bunch of wrapper classes. Perhaps an area of future work.

Any strategy at some point will need to read the current market data, for that there is the AssetReadNode:


```cpp
export class AssetReadNode final
	: public ExpressionNode<LinAlg::EigenConstColView<double>>
{
private:
	size_t m_column;
	int m_row_offset;
	size_t m_warmup;
	size_t m_null_count = 0;
	Exchange const& m_exchange;

public:
	AssetReadNode(size_t column, int row_offset, const Exchange& exchange
	) noexcept
		: ExpressionNode<LinAlg::EigenConstColView<double>>(NodeType::ASSET_READ),
		m_column(column),
		m_row_offset(row_offset),
		m_exchange(exchange),
		m_warmup(static_cast<size_t>(std::abs(m_row_offset))) {}

		LinAlg::EigenConstColView<double> evaluate() noexcept override;

		...
```

The asset read node inherits from the ExpressionNode which is a templated abstract base class whose main method is the pure virtual function evaluate(). In this case we take in the column we went to read, an appropriate index offset, and return a view into the appropriate Exchange's data matrix at the given location. The exchange handles the tricky indexing, all we pass to it is the relative column index we want, and the relative row offset.

```cpp
LinAlg::EigenConstColView<double>
Exchange::getSlice(size_t column, int row_offset) const noexcept
{
	size_t idx = ((m_impl->current_index - 1) * m_impl->col_count) + column;
	if (row_offset)
	{
		idx -= abs(row_offset) * m_impl->col_count;
	}
	return m_impl->data.col(idx);
}

```

To opperate on the data, we have the AssetOpNode which again is a templated child class of the expression node and is used to generate four class, one for each basic arithmatic type: +,-,/,*. The input to this class is a left and right child node that each represent a read operation into the exchange's data, in past designs the child nodes themselves were allowed to be op nodes but I have yet to implement that in Atlas. 
```cpp
export template <typename NodeCwiseBinOp>
	class AssetOpNode
	:public ExpressionNode<LinAlg::EigenCwiseBinOp<NodeCwiseBinOp>>
{
private:
	SharedPtr<AssetReadNode> m_asset_read_left;
	SharedPtr<AssetReadNode> m_asset_read_right;
	AssetOpType m_op_type;
	size_t warmup;

public:
	virtual ~AssetOpNode() noexcept = default;

	AssetOpNode(
		SharedPtr<AssetReadNode> asset_read_left,
		SharedPtr<AssetReadNode> asset_read_right
	)
```

At runtime, each asset op node returns and Eigen cwise binary operation that has yet to be evaluated. For example, a moving average crossover strategy could read in the current close, subtract the current n period moving average, and then allocate accordingly which we will look at next.

```cpp
[[nodiscard]] LinAlg::EigenCwiseBinOp<NodeCwiseBinOp>
	evaluate() noexcept override
{
	auto left_view = m_asset_read_left->evaluate();
	auto right_view = m_asset_read_right->evaluate();

	if constexpr (std::is_same_v<NodeCwiseBinOp, LinAlg::EigenCwiseProductOp>)
	{
		return left_view.cwiseProduct(right_view);
	}

	else if constexpr (std::is_same_v<NodeCwiseBinOp,LinAlg::EigenCwiseQuotientOp>)
	{
	return left_view.cwiseQuotient(right_view);
	}
	...
```

After the operation is executed in the ExchangeViewNode (which I will skip here), an allocation is generated by and AllocationNode. There are currently two allocation types supported: UNIFORM, and CONDITIONAL_SPLIT. Uniform simply allocated 1/n weight to each element in the exchange view that is a valid number ( not Nan). Conditional split allocates -(1/N) to all values in the exchange view that are less then some value, and (1/N) to those greater.

```cpp
export class AllocationNode
	final
	: public OpperationNode<void, Eigen::VectorXd&>
{
private:
	SharedPtr<ExchangeViewNode> m_exchange_view;
	Exchange& m_exchange;
	double m_epsilon;
	AllocationType m_type;
	Option<double> m_alloc_param;

public:
	ATLAS_API ~AllocationNode() noexcept;
	ATLAS_API AllocationNode(
		SharedPtr<ExchangeViewNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;

	void evaluate(Eigen::VectorXd& target) noexcept override;
```

Here we see an example of the allocation node, which takes in a single parameter, a reference to an dynamiclly sized eigen vector of doubles representing the strategies target weights, and attempts to allocate them in place. The allocation is actually quite simple, as we can take advantage of the eigen api to allocate the strategy extremely quickly.

```cpp
void
AllocationNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// evaluate the exchange view to calculate the signal
	m_exchange_view->evaluate(target);

	// calculate the number of non-NaN elements in the signal
	size_t nonNanCount = target.unaryExpr([](double x) { return !std::isnan(x) ? 1 : 0; }).sum();
	double c;
	if (nonNanCount > 0) {
		c = (1.0 - m_epsilon) / static_cast<double>(nonNanCount);
	}
	else {
		c = 0.0;
	}

	switch (m_type) {
	case AllocationType::UNIFORM: {
		target = target.unaryExpr([c](double x) { return x == x ? c : 0.0; });
		break;
	}
	case AllocationType::CONDITIONAL_SPLIT: {
		target = (target.array() < *m_alloc_param)
			.select(-c, (target.array() > *m_alloc_param)
				.select(c, target));
		target = target.unaryExpr([](double x) { return x == x ? x : 0.0; });
		break;
	}
	}
}
```

The allocation vector is first passed to the exchange view where the signal is generated in place to save memory and allocations. Then all we have to do is calculate the number of non-NaN elements in the signal, and that use a unary expression to evaluate the target allocations in place using their respective index in the vector as the respective index of the asset to allocated.

Expressing the portfolio as a series of target weights allows to evaluate the portfolio extremly easily. All we have to do is take the sum product of the target weights, and the asset returns (pre computed at exchange intialization) to find the portfolio return at time t. Putting it all together: 

```cpp
void
Strategy::evaluate() noexcept
{
	// get the current market returns
	LinAlg::EigenConstColView market_returns = m_impl->m_exchange.getMarketReturns();

	// get the portfolio return by calculating the sum product of the market returns and the portfolio weights
	double portfolio_return = market_returns.dot(m_impl->m_target_weights_buffer);

	// update the tracer nlv 
	double nlv = m_impl->m_tracer.getNLV();
	m_impl->m_tracer.setNLV(nlv * (1.0 + portfolio_return));
	m_impl->m_tracer.evaluate();
}


void
Strategy::step() noexcept
{
	if (!m_step_call)
	{
		return;
	}
	// check if warmup over 
	if (m_impl->m_exchange.currentIdx() < m_impl->m_ast->getWarmup())
	{
		return;
	}
	// exeucte ast
	m_impl->m_ast->evaluate(m_impl->m_target_weights_buffer);
	m_step_call = false;
}
```

Finally, we have a strategy and simulation manager class Hydra that is responsible for managing the exchanges and strategies. During a simulation it loops there the sorted union of datetimes and calls strategy evaluation accordingly.

```cpp
void
Hydra::step() noexcept
{
	m_impl->m_exchange_map.step();
	
	for (int i = 0; i < m_impl->m_strategies.size(); i++)
	{
		m_impl->m_strategies[i]->evaluate();
		m_impl->m_strategies[i]->step();
	}
}


void
Hydra::run() noexcept
{
	if (m_state == HydraState::FINISHED)
	{
		auto res = reset();
	}
	size_t steps = m_impl->m_exchange_map.getTimestamps().size();
	for (size_t i = 0; i < steps; ++i)
	{
		step();
	}
	m_state = HydraState::FINISHED;
}
```

Each step in the simulation involves moving listed exchanges forward in time by one step if their respective next time is equal to the next global time, evaluating the strategies with their current positions, and the allowing the strategies to execute their logic. As of now the strategy class is a concrete class made up of a AllocationNode as described earlier. Past iterations of this project have allowed the step function to be virtual to allow for more complex strategies which I plan on including in the future.

With that being said, I wanted the framework to be "pay for what you use". I.e. it should be extremly fast at simulating simple vectorized strategies, but allow for more complex event driven trading strategies if you wish. With that in mind, we turn now to AtlasPy, the pybind11 wrapper that exposes the C++ engine to python.

## Python Wrapper

The enitre python wrapper sits in a single module.cpp file in it's own visual studio project. It exports a .pyd python extension that is specific to the operating system and python version pybind11 used to compile it. The code is fairly straight forward so I'll only include a small snippet as an example: 

```cpp
PYBIND11_MODULE(AtlasPy, m) {
    auto m_core = m.def_submodule("core");
    auto m_ast = m.def_submodule("ast");
    
	...
	    py::class_<Atlas::Strategy, std::shared_ptr<Atlas::Strategy>>(m_core, "Strategy")
        .def("getNLV", &Atlas::Strategy::getNLV)
        .def("getName", &Atlas::Strategy::getName)
        .def("enableTracerHistory", &Atlas::Strategy::enableTracerHistory)
        .def("getHistory", &Atlas::Strategy::getHistory, py::return_value_policy::reference_internal)
        .def(py::init<std::string, std::shared_ptr<Atlas::AST::StrategyNode>, double>());

	...
```

Above we see the wrapper for the Strategy class with a couple of interesting things. We see the first line define the Strategy class and note that it returns a shared pointer. The last line defines the python constructor, that takes in a string id, a shared pointer to a AST strategy node, and then a double representing the portfolio % allocation to this strategy. The getHistory function returns a const reference to an eigen vector as a read only and non-owning numpy array which helps reduce copies in the interop layer. I've tried wrapping C++ with ctypes lib before, and can say pybind11 is a huge improvement, if you want to check out this source you can find it at: https://github.com/ntorm1/Atlas/blob/main/AtlasPy/src/module.cpp.

Now lets see how fast our engine is after being exposed to a dynamically typed language. Our first comparison will be agaianst VectorBT, an extremlely fast vectorized backtesting framework built on numpy and Numba. I think the package gets a lot of things right, but my main gripe being once you move into more complex strategies it becomes extremly confusing in my opinion, with multiindexed dataframes, and massive class instances. I found building anything beyond a single asset test from signals to be confusing, so that is what I implement here directly from their documention:

```python
import vectorbt as vbt

start = '2019-01-01 UTC'  # crypto is in UTC
end = '2020-01-01 UTC'
btc_price = vbt.YFData.download('BTC-USD', start=start, end=end).get('Close')

fast_ma = vbt.MA.run(btc_price, 10, short_name='fast')
slow_ma = vbt.MA.run(btc_price, 20, short_name='slow')


time_sum = 0
n = 100
for i in range(n):
    st = time.perf_counter_ns()
    entries = fast_ma.ma_crossed_above(slow_ma)
    exits = fast_ma.ma_crossed_below(slow_ma)
    pf = vbt.Portfolio.from_signals(btc_price, entries, exits)
    et = time.perf_counter_ns()
    time_sum += et - st

avg_time_millis = time_sum / n / 1e6
tr = pf.total_return()

print(f"Total return: {tr:.3%}")
print(f"Time elapsed Avg: {avg_time_millis:.3f} ms")
"""
Total return: 63.519%
Time elapsed Avg: 18.029 ms
"""

```

To simulate a simple moving strategy crossover over one year of daily data takes on overage 18 ms, a very respectable time. Note that I don't include the data load time, or the time taken to calculate features, just the generation of signals and execution of the strategy. VectorBT has much morer functionality around that space that I have not implemented in Atlas that makes their library much more usable, so take that for what it is worth. Now lets run the same strategy in AtlasPy to check we get the same return. First let's set up the Atlas runtime and objects, we start by adding the path to the .pyd extension to the system path list so we can import AtlasPy, then load in the required elements.

```python
atlas_path = "C:/Users/ ... /Atlas/x64/Release"
exchange_path_fast = "C:/Users/ ... /Atlas/AtlasPy/test/data_fast.h5
sys.path.append(atlas_path)

from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast import AssetReadNode, AssetDifferenceNode, \
      ExchangeViewNode, AllocationNode, StrategyNode, AssetOpNodeVariant, ExchangeViewFilterType

exchange = hydra.addExchange("test", exchange_path_fast)
portfolio = hydra.addPortfolio("test_p", exchange, 100.0)
hydra.build()

```

Now we can create our moving crossover strategy using our Abstract Strategy Tree. At each time step take the fast moving average and subtract from it the slow moving average, and then filter out entries less than 0.0

```python
read_fast = AssetReadNode.make("ma_fast", 0, exchange)
read_slow = AssetReadNode.make("ma_slow", 0, exchange)
spread = AssetDifferenceNode(read_fast, read_slow)
op_variant = AssetOpNodeVariant(spread)

exchange_view = ExchangeViewNode(exchange, op_variant)
exchange_view.setFilter(ExchangeViewFilterType.GREATER_THAN, 0.0)
allocation = AllocationNode(exchange_view)
strategy_node = StrategyNode(allocation, portfolio)
strategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, 1.0))
```

Finally lets run the simulation and time the execution speed

```python
time_sum = 0
n = 100
for i in range(n):
    st = time.perf_counter_ns()
    hydra.run()
    et = time.perf_counter_ns()
    time_sum += et - st

avg_time_micros = (time_sum / n) / 1000

tr = (strategy.getNLV() - initial_cash) / initial_cash
print(f"Time elapsed Avg: {avg_time_micros:.3f} us")
print(f"Total return: {tr:.3%}")
print(f"Epsilon: {tr - 0.6351860771192923}")

"""
Time elapsed Avg: 16.951 us
Total return: 63.519%
Epsilon: 1.7763568394002505e-15
"""
```

Atlas can run the same simulation in 17 microsecond, compared to 18 milliseconds taken by VectorBT a speed up factor of over 1000x. Note of course this is not an apples to apples comparison, I believe VectorBT includes run time checks on cash levels and such, as well as most likely has some behind the scenes allocations to store historical orders. Additionally, I am by no means an expert in VectorBT so if there is a faster way to do this then let me know, but for now I am happy with the results.

Now lets look at a more complex example, involving multiple assets and shorting as well. For this I am using the bt python library (https://github.com/pmorissette/bt) as I find VectorBT to be a pain in the ass for simulations like this. Lets run a 50-200 period moving average strategy over a list of 14 different stocks in the SP500, first we use bt to download our data:

```python
tickers_new = 'aapl,msft,c,gs,ge,jnj,pg,ko,amzn,jpm,adbe,ma,dis,txn'
data = bt.get(tickers_new, start='2010-01-01')
sma50 = data.rolling(50).mean()
sma200 = data.rolling(200).mean()
data.shape
```

Then we use their API to define a strategy that goes long each stock whose fast ma is above their slow, and short the opposite, applying equal weight to each. The code below is largely taken from their examples page in the documentation:

```python
class WeighTarget(bt.Algo):
    def __init__(self, target_weights):
        self.tw = target_weights

    def __call__(self, target):
        if target.now in self.tw.index:
            w = self.tw.loc[target.now]
            target.temp['weights'] = w.dropna()

        return True
    
st = time.time()

## now we need to calculate our target weight DataFrame
# first we will copy the sma200 DataFrame since our weights will have the same strucutre
tw = sma200.copy()

# set appropriate target weights
tw[sma50 > sma200] = 1.0
tw[sma50 <= sma200] = -1.0

# divide by row sum to normalize
tw = tw.div(tw.abs().sum(axis=1), axis=0)

# replace nans with 0.0
tw[sma200.isnull()] = 0.0

ma_cross = bt.Strategy('ma_cross', [WeighTarget(tw),
                                    bt.algos.Rebalance()])

t = bt.Backtest(ma_cross, data)
res = bt.run(t)
et = time.time()
print(f"Run time: {et-st:.2f} seconds")
res.display()

"""
Run time: 2.25 seconds
Stat                 ma_cross
-------------------  ----------
Start                2010-01-03
End                  2024-01-19
Risk-free rate       0.00%

Total Return         110.16%
...
"""
```

Now lets look at the same strategy in Atlas. In the AST all we have to do is read in the fast ma, subtract from it the slow moving average, and then use a conditional split allocation node to go long or short an equal weight accordingly.

```python

slow_ma = AssetReadNode.make("slow_ma", 0, exchange)
fast_ma = AssetReadNode.make("fast_ma", 0, exchange)
spread = AssetDifferenceNode(fast_ma, slow_ma)
op_variant = AssetOpNodeVariant(spread)

exchange_view = ExchangeViewNode(exchange, op_variant)
allocation = AllocationNode(exchange_view, AllocationType.CONDITIONAL_SPLIT, 0.0, 0.0)
strategy_node = StrategyNode(allocation, portfolio)
strategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, 1.0))
strategy.enableTracerHistory(TracerType.NLV)

st = time.perf_counter_ns()
hydra.run()
et = time.perf_counter_ns()
time_micros = (et - st) / 1000

tr = (strategy.getNLV() - initial_cash) / initial_cash
print(f"Time elapsed Avg: {time_micros:.3f} us")
print(f"Total return: {tr:.3%}")
print(f"Epsilon: {tr - 1.1016}")
"""
Time elapsed Avg: 300.315 us
Total return: 110.305%
Epsilon: 0.0014539039503378426
"""
```

![Svelte](ma_cross_returns.png)

We see here we are off by .14%, due to floating point rounding errors most likely. You can try for yourself and see the equity curves are indistinguishable. You'll also note Atlas is able to run it in 300 microseconds, compared to the 2.25 seconds of bt, a speed up of 7500x, not to shabby. It was able to process 44,030 candles in 300 microseconds, for a throughput of almost 150 million candles per second on a single thread. This is a toy example, but if you extend this example to the entire SP500 you'll see why Atlas is useful. You can execute the strategy in the order of milliseconds rather than minutes.

Again, not exactly an apples to apples comparison, bt and VectorBT are most likely doing things at run time behind the scense that Atlas is not. For example, if you include the AST creation in the runtime calculation the elapsed time jumps to 500 microseconds. Additionally, Atlas is far from usable, it has no indicator calculations and can only be read in from an h5 file. Making the library useable in that sense is my next target, as long as providing vol scaling strategies using inverse vol weighting and equal risk contribution, as well as enable multithreading to really juice Atlas's performance. The source code for Atlas can be found at https://github.com/ntorm1/Atlas/tree/main, including all code used to generate these outputs.