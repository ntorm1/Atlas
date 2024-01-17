// AtlasPerf.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#pragma warning(suppress:5050)
import HydraModule;
import ExchangeModule;
import PortfolioModule;
import StrategyModule;
import TracerModule;

import AssetNodeModule;
import ExchangeNodeModule;
import StrategyNodeModule;

#include <iostream>

using namespace Atlas;
using namespace Atlas::AST;

std::shared_ptr<Atlas::Hydra> hydra = std::make_shared<Hydra>();
std::string exchange_id = "test";
std::string portfolio_id = "test_p";
std::string strategy_id = "test_s";
std::string exchange_path_sp500 = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasTest/scripts/data_sp500.h5";
double initial_cash = 100.0f;

auto exchange = hydra->addExchange(exchange_id, exchange_path_sp500).value();
auto portfolio = hydra->addPortfolio(portfolio_id, *exchange, initial_cash).value();


int main()
{
	hydra->build();
	auto const exchange = hydra->getExchange("test").value();
	auto read_close = AssetReadNode::make("close", 0, *exchange);
	auto read_50_ma = AssetReadNode::make("50_ma", 0, *exchange);
	auto daily_return = AssetDifferenceNode::make(
		std::move(*read_close),
		std::move(*read_50_ma)
	);
	auto op_variant = AssetOpNodeVariant(std::move(daily_return));
	auto exchange_view = ExchangeViewNode::make(
		*exchange, std::move(op_variant)
	);
	exchange_view->setFilter(ExchangeViewFilterType::GREATER_THAN, 0.0f);
	auto alloc = AllocationNode::make(std::move(exchange_view));
	auto strategy_node = StrategyNode::make(std::move(*alloc), *portfolio);
	auto strategy = std::make_unique<Strategy>(
		strategy_id,
		std::move(strategy_node),
		1.0f
	);
	auto res = hydra->addStrategy(std::move(strategy));
	int n = 10000;
	std::cerr << "Running" << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < n; i++)
	{
		hydra->run();
		hydra->reset();
	}
	auto end = std::chrono::high_resolution_clock::now();
	// Calculate total time
	auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	// Calculate average time per iteration
	auto averageTimePerIter = totalTime / n;

	// Print results
	std::cout << "Total time: " << totalTime << " microseconds\n";
	std::cout << "Average time per iteration: " << averageTimePerIter << " microseconds\n";

	return 0;
}

