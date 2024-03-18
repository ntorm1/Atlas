---
title: Building an Algorithmic Trading IDE
description: Vectorized SL/TP, Cross Sectional Rankings, and an Expiremtnal GUI
date: '2024-1-28'
categories:
  - Atlas
published: true
---

#### Building an Algorithmic Trading IDE

## Table of Contents

## Intro
I wanted to make an update to my post from last week with some updates to both the backtesting engine, as well as a look at an experimental GUI interface that allows for more rapid devolpment for trading strategies. The first post did fairly well I guess? It got 1500 views as of this writing after I posted it to the algo trading and CPP sub reddits. Anyway, I added a lot of changes over the past week so lets take a look first at the C++ and the underlying engine.

## Commissions
The first thing I wanted to do was add a way to adjust for commissions, both on a fixed term (i.e. $10 per order), or on a notional percentage amoubt (20 bps of total trade value). In order to do this I first had to implement functionality to adjust for the fact that after you set the target weights at time t0, these weights will change due to natural variation in asset returns by time t1 if not adjusted. Thus even if you set your portfolio weights the same, you will have implicity traded, we have to account for that. Do this, we monitor strategy evaluation as defined below:


```cpp
void
Strategy::lateRebalance() noexcept
{
	// if the strategy does not override the target weights buffer at the end of a time step,
	// then we need to rebalance the portfolio to the target weights buffer according to the market returns
	LinAlg::EigenConstColView market_returns = m_impl->m_exchange.getMarketReturns();

	// update the target weights buffer according to the indivual asset returns
	Eigen::VectorXd returns = market_returns.array() + 1.0;
	m_impl->m_target_weights_buffer = returns.cwiseProduct(m_impl->m_target_weights_buffer);

	// divide the target weights buffer by the sum to get the new weights
	auto sum = m_impl->m_target_weights_buffer.sum();
	if (sum > 0.0)
	{
		m_impl->m_target_weights_buffer /= sum;
	}
	assert(!m_impl->m_target_weights_buffer.array().isNaN().any());
}
```
https://github.com/ntorm1/AgisCore2-/blob/main/modules/strategy/Strategy.cpp

Directly after a strategy's ast is executed, we check to see if the weights were adjusted directly. If the weights were not adjusted then we have to update the actual realized allocations seen in the code above, simply but updateing the weights accordingy to the market returns. Note that this does slow down tighter simulations, so I include the ability to override it. If your strategy promises to set new weights each step and you have no commissions, the late rebalance has no effect. Before we get to commissions, we note here this also allows for trigger based strategies that are designed to fire with a given frequency, i.e. monthly.

To implement this, we have to do is define the frequency, evaluate the strategy at the given point in time, the the late rebalance will do all the work for us in terms of naturally adjusting the weights of the portfolio due to the lack of forced rebalancing. To do this we define a fixed allocation node that operates over a vector of pairs representing allocations and evaluate accordingly when triggered. The nice thing being the vector of pairs is that it can be passed directly into C++ via pybind11 using a list of tuples and pybind11's stl header file.

```cpp
//============================================================================
export class FixedAllocationNode final
	: public AllocationBaseNode
{
private:
	Vector<std::pair<size_t, double>> m_allocations;

public:
    void evaluateChild(LinAlg::EigenVectorXd& target) noexcept override;

    ...
    ...

void
FixedAllocationNode::evaluateChild(Eigen::VectorXd& target) noexcept
{
	for (auto& pair : m_allocations)
	{
		assert(pair.first < static_cast<size_t>(target.size()));
		target(pair.first) = pair.second;
	}
}
```

Looking now as to how to actually evaluate commissions. To do this we define a Allocation base node that is responsible for calling the child evaluation logic, as seen above, as well as keeping track of the previous timesteps portfolio allocation. This takes place in it's respective evaluate class, and has 5 steps seen in the code below.

1. The taget, which is a reference to an eigen column, currently has the prior timesteps weights in it. So if needed we take advatnage of that any copy over the target into a seperate buffer to prevent overwrrite on strategy evaluation.

2. Evalaut child, call the actual evaluation which executes core strategy/allocation logic and operates over the target buffer.

3. Check member epsilon. If it is defined, and allocation whose new target weight is less than epsilon % away from the current weights is reset to the current weights. This is a handy trick to cut down on turnover and commissions when using strategies that using weighting algorithms like vol scaling.

4. Evaluate option stop loss and take profit (more on this latter)

5. Calculation of the actual commissions using the new target weights and the prior weights.



```cpp
void
AllocationBaseNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// if we have a commission manager or trade watcher we need to copy the current weights buffer 
	// into the commission manager buffer before it gets overwritten by the ast. 
	// 
	// Additionally if weight epsilon is set we need to copy the current weights
	// so that any adjustments made are reverted back if the absolute value of the
	// difference is less than epsilon
	if (
		m_impl->m_commision_manager||
		m_impl->m_epsilon ||
		m_impl->m_trade_limit
		)
	{
		m_impl->m_weights_buffer = target;
	}

	evaluateChild(target);

	// if epsilon is set we need to check if the difference between the current
	// weights and the new weights is less than epsilon. If it is we need to
	// revert the weights back to the original weights before calculating any commissions
	if (m_impl->m_epsilon)
	{
		target = ((target - m_impl->m_weights_buffer).cwiseAbs().array() < m_impl->m_epsilon)
			.select(m_impl->m_weights_buffer, target);
	}

	// if we have stop loss or take profit we need to check if the trade limits have been exceeded
	// by passing in the previous weights to compute the pnl, then adjust target weights accordingly
	if (m_impl->m_trade_limit)
	{
		(*m_impl->m_trade_limit)->evaluate(target, m_impl->m_weights_buffer);
	}

	// if we have a commission manager we need to calculate the commission caused
	// by the current weights adjustment
	if (m_impl->m_commision_manager && (*m_impl->m_commision_manager)->hasCommission())
	{
		(*m_impl->m_commision_manager)->calculateCommission(target, m_impl->m_weights_buffer);
	}
}

```

Calculting the commissions is then fairly straight forward (though I confess I have not extensively tested it.) Simply evaluate two kinds of commissions: fixed and percentage. The sum of these two is then subtract from the nlv of the strategy.

```cpp
double
CommisionManager::calculateFixedCommission(
	Eigen::VectorXd const& target_weights,
	Eigen::VectorXd const& current_weights
	) noexcept
{
	// get a count of number of weights that are different
	size_t count = (target_weights - current_weights)
		.unaryExpr([](double x) { return std::abs(x) < ORDER_EPSILON ? 0 : 1; })
		.sum();

	return count * (*m_fixed_commision);
}
double
CommisionManager::calculatePctCommission(
	double nlv,
	Eigen::VectorXd const& target_weights,
	Eigen::VectorXd const& current_weights) noexcept
{
	// get a count of number of weights that are different
	return ((target_weights - current_weights).cwiseAbs()
		* nlv * m_commission_pct.value()).sum();
}
```

## Stop Loss / Take Profit

Expanding on what was said above, we can use the allocation evaluation to compare the prior period's weights to the target weights as well as we need a way to keep track of open trades current p/l. The evaluation function is somewhat complex with lots of comments so I break it down into two chunks.

The first chunk is responsible for mainting the P/L buffer at every time stamp, by doing a cwise produce on the prior pnl and 1 + previous returns. We'll see this in the next chunk, but the pnl vector is intalized to 1 when a new trade is opened, the simplyfing the pnl calculations as a percentage opperation. I.e. element at index i starts with 1 at time t1, but at time t2 asset i's return was -10%, so element at index i now holds .9. This pnl vector is then compared with the threshold to zero out any weights exceeding. 

```cpp
//============================================================================
void TradeLimitNode::evaluate(
	LinAlg::EigenVectorXd& current_weights,
	LinAlg::EigenVectorXd& previous_weights
) noexcept
{
	// in the first step we can't have pnl as allocation node is evaluated after the 
	// portfolio is priced. And it would cause index error trying to get the previous steps
	// returns as we see below.

	if (m_is_first_step)
	{
		m_is_first_step = false;
	}
	else
	{
		// Trade limit node evaluate is called before the allocation node updates.
		// Therefore, current_weights hold the weights that were used for the most 
		// recent evaluation. So we need to update the trade pnl vector to adjust the pnl
		// Stat by pulling in the previous market returns using index offset and making a mutable copy of the view
		Eigen::VectorXd previous_returns = m_exchange.getMarketReturns();
		previous_returns.array() += 1.0;

		// multiply 1 + returns to get the new price using the m_pnl buffer
		m_impl->m_pnl = m_impl->m_pnl.cwiseProduct(previous_returns);

		// switch on the trade type to zero out weights as required. For stop loss
		// we zero out the weights when the pnl is less than the limit. For take profit
		// we zero out the weights when the pnl is greater than the limit. Add an additional
		// comparison to 0.0 to prevent the new weights from being zeroed out beforehand
		switch (m_impl->m_trade_type)
		{
		case TradeLimitType::STOP_LOSS:
			current_weights = current_weights.array().cwiseProduct(
				((m_impl->m_pnl.array() > m_impl->m_limit) || (m_impl->m_pnl.array() == 0.0)).cast<double>()
			);
			break;
		case TradeLimitType::TAKE_PROFIT:
			current_weights = current_weights.array().cwiseProduct(
				((m_impl->m_pnl.array() < m_impl->m_limit) || (m_impl->m_pnl.array() == 0.0)).cast<double>()
			);
			break;
		}
	}
```

For this to work we have to observe any new trades, defined either a new postion, or a reversal in sign. To do this we use some Eigen magic (probably can be condensed down but it works in the limited test I have). 


```cpp
	// init the pnl trade vector to 1 where the trade pct switched sign or 
	// went from 0 to non-zero
	m_impl->m_pnl = (
		(current_weights.array() * previous_weights.array() < 0.0f)
		||
		(previous_weights.array() == 0) && (current_weights.array() != 0)
		)
		.select(1.0f, m_impl->m_pnl);

	// update closed trade. If the current weight is 0 and the previous weight is not 0
	// then zero out the pnl vector
	m_impl->m_pnl = (
		(current_weights.array() == 0)
		&& 
		(previous_weights.array() != 0)
		)
		.select(0.0f, m_impl->m_pnl);

```
https://github.com/ntorm1/Atlas/blob/main/modules/ast/TradeNode.cpp

Note two limitations: It does not track trade adjustments, just raw percentage returns of the underlying asset, and can Only have a TP or SL (will fix this soon). Now we can look at cross sectionally ranking.

## Ranking
Many equity algo strategies opperate on large asset universe and operate by evaluating some function on each asset then ranking the output of each. For example look at the one month return and buy the top 10% and short the buttom 10%. To do this we use a simple ranking node, for now we can get the N largest, the N smallest,
or N Extreme, which returns the N smallest in the first N elements of the buffer, and the N largest in the last N elements of the buffer.

```cpp
export enum class EVRankType : uint8_t
{
	NLargest,	
	NSmallest,	
    NExtreme, 
};

export class EVRankNode final : public StrategyBufferOpNode
{
private:
	size_t m_N;
	EVRankType m_type;
	UniquePtr<ExchangeViewNode> m_ev;
	Vector<std::pair<size_t, double>> m_view;
	void sort() noexcept;
public:
	ATLAS_API EVRankNode(
		UniquePtr<ExchangeViewNode> ev,
		EVRankType type,
		size_t count
	) noexcept;
	ATLAS_API ~EVRankNode() noexcept;
	size_t getWarmup() const noexcept override {return 0;}
	void evaluate(Eigen::VectorXd& target) noexcept override;

```

The rank node operates after the exchange view has evaluated and populated the target buffer with the desired calculations. Finding the N largest or smallest elements is a bit tricky, as we need their actual indecies not just the values. To do this we maintain a vector of pairs between asset index and actual value calculated by the exchange view. I'll leave out the sort function now (it's a fairly straight forward partial sort, the N extreme is slightly more interesting.)

```cpp
//============================================================================
void
EVRankNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// before executing cross sectional rank, execute the parent exchange 
	// view operation to populate target vector with feature values
	m_ev->evaluate(target);

	// then we copy target over to the view pair vector to keep 
	// track of the index locations of each value when sorting
	assert(static_cast<size_t>(target.size()) == m_view.size());
	for (size_t i = 0; i < m_view.size(); ++i)
	{
		m_view[i].second = target[i];
	}

	// sort the view pair vector, the target elements will be in
	// the first N locations, we can then set the rest to Nan to
	// prevent them from being allocated. At which point we have a 
	// target vector that looks like: 
	// [Nan, largest_element, Nan, second_largest_element, Nan, Nan]
	sort();
    switch (m_type) {
        case EVRankType::NSmallest:
        case EVRankType::NLargest:
            for (size_t i = 0; i < m_N; ++i)
            {
                target[m_view[i].first] = m_view[i].second;
            }
            for (size_t i = m_N; i < m_view.size(); ++i)
            {
				target[m_view[i].first] = std::numeric_limits<double>::quiet_NaN();
			}
			break;
        case EVRankType::NEXTREME:
            assert(false); // not implemented
    }
}
```
https://github.com/ntorm1/Atlas/blob/main/modules/ast/RankNode.cpp

To take advantage of these we need to add new allocation types. Right now we have uniform which sets all non-nan elements to 1/n weight and conditional split which sets elements less than X to -1/n and vice versa. To take advantage of the ranking we need to add additional allocation methods. The simplist are NLargest and NSmallest which simply call through to uniform to allocate to non-Nan elements. NExtreme will need some work, as there is no way to differentiate between the two with the current design.


We now look at a new subproject of Atlas: AtlasX.

## AtlasX

AtlasX is a qt based GUI application that is the second iteration I have made and is still in the early stages. As of now, you can add exchanges, portfolios, and strategies, and the run simulations all withen the GUI. Below is a screenshot of the current state (couldn't get screenshot to work on my monitor so it's squeezed into my laptop screen.)

![Svelte](AtlasX.png)


There is quite a bit of code behind the scense, some of which I reused from previous project. The main featues are QT Advanced docking system to get a docking manager. Each panel can be resized, tabbed, or made into it's own floating screen. The text editor is based of QScintilla and has a simple auto complete functionality. I'd like to hook it up to python stub files from AtlasPy to get actual auto complete and LSP integration.

You can see you can also take a step forward in time rather than running the entire code. This allows you to see how the portfolio and assets change over time. Soon to add live trade and order tables but this will be tricky as we are not using an event based approach and will have to work back from an Eigen matrix of historical portfolio weights to deduce trades and orders. Hopefully can complete that this week.

The main feature is integration with pybind11 to embed the python interpreter in the Qt Application so we can 'compile' the python code defining the strategy into the abstract strategy tree. This main functionality is seen below, I remove at lot of error handeling and set up but you get the idea.

```cpp
//============================================================================
void
AtlasXStrategyManager::compileStrategy() noexcept
{
	m_impl->editor->forceSave();

    // load the strategy file parent dir into the sys path and import it
	String const& py_path = m_impl->strategy_temp.value()->path;
	fs::path py_dir = fs::path(py_path).parent_path();
	appendIfNotInSysPath(py_dir.string());

    // load the module into the cache as it does not yet exsist and execute build func
	String const& strategy_name= m_impl->strategy_temp.value()->strategy_name;
    m_impl->modules[strategy_name] = py::module::import(
        (m_impl->strategy_temp.value()->strategy_name).c_str()
    );
    py::object result = m.attr("build")(app->getHydra());

    ...


}
```

The main idea is as follow.

1. To trigger compile you can click on the terminal widget on the right side of the strategy manager. Doing so will force QScintilla editor to save any changes.

2. We then append the system path with the parent directory of the strategy file. All files are auto generated and managed by the app (more below). 

3. Then we import the .py file by the strategy name and execute the build function. This function as seen above is expected to take the hydra shared pointer as the parameter and result and a call to hydra.addStrategy() to register the new instance. Thus after the build function runs, we have changed the state of the hydra instance and now contain a new strategy as defined by the python function. 

It still needs some work on the error handeling, but functionly it works and allows for dynamic strategy creation and a extremly integrated interface. New strategies start with auto generated files defining the template to expect.

```cpp
void
AtlasXStrategyManager::openStrategy() noexcept
{
	auto const& env_path_str = m_impl->app->getEnvPath();
	fs::path env_path(env_path_str);
	fs::path strategy_dir = env_path / "strategies";
	if (!fs::exists(strategy_dir)) 
	{
		fs::create_directory(strategy_dir);
	}

	fs::path strategy_py = strategy_dir / (m_impl->strategy_temp.value()->strategy_name + ".py");
	if (!fs::exists(strategy_py))
	{
		String alloc = std::to_string(m_impl->strategy_temp.value()->portfolio_alloc);
		std::ofstream strategy_file(strategy_py);
		strategy_file << "from AtlasPy.core import Strategy\n";
		strategy_file << "from AtlasPy.ast import *\n";
		strategy_file << "\n\n";
		strategy_file << "strategy_id = \"" << m_impl->strategy_temp.value()->strategy_name << "\"\n";
		strategy_file << "exchange_id = \"" << m_impl->strategy_temp.value()->exchange_name << "\"\n";
		strategy_file << "portfolio_id = \"" << m_impl->strategy_temp.value()->portfolio_name << "\"\n";
		strategy_file << "alloc = " << alloc << "\n\n";
		strategy_file << "def build(hydra):\n";
		strategy_file << "\texchange = hydra.getExchange(exchange_id)\n";
		strategy_file << "\tportfolio = hydra.getPortfolio(portfolio_id)\n\n\n";
		strategy_file << "\tstrategy = hydra.addStrategy(Strategy(strategy_id, strategy_node, alloc))\n";
		strategy_file.close();
	}
	QString strategy_path = QString::fromStdString(strategy_py.string());
	m_impl->editor->loadFile(strategy_path);
	m_impl->strategy_temp.value()->path = strategy_path.toStdString();
}
```
https://github.com/ntorm1/Atlas/blob/main/AtlasX/src/AtlasXStrategyManager.cpp

Here we see what happens when we create / open a strategy. We either load the existing py file directly into the editor, or we generate a template based on the strategy managers current settings as defined by the user. 

There is a lot of cool functionality arround other parts of the app, including live charting, serialization and deserializtion of hydra instances and more. That beind said there is a fair bit of boiler plate code and replication. To avoid visual studio inteli-sense failing AtlasX does not used modules, and all interface with the Atlas.dll runtime is hidden which makes everythting a lot harder. For now I leave it there. Hopefully can implement the next set of features in Atlas and then port their needed implementation to AtlasX for easy testing and use, to be seen.