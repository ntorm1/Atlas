

#include "helper.h"


import HydraModule;
import ExchangeModule;
import PortfolioModule;
import AtlasTimeModule;

import RiskNodeModule;
import HelperNodesModule;

using namespace Atlas;
using namespace Atlas::AST;
using namespace Atlas::Time;



class RiskTests : public ::testing::Test
{
protected:
	std::shared_ptr<Atlas::Hydra> hydra;
	std::shared_ptr<Atlas::Portfolio> portfolio;
	std::shared_ptr<Atlas::Exchange> exchange_sptr;
	std::string exchange_id = "test";
	std::string portfolio_id = "test_p";
	std::string strategy_id = "test_s";
	double initial_cash = 100.0f;

	void SetUp() override
	{
		hydra = std::make_shared<Hydra>();
		exchange_sptr = hydra->addExchange(exchange_id, exchange_path_sp500).value();
		portfolio = hydra->addPortfolio(portfolio_id, *(exchange_sptr.get()), initial_cash).value();
	}
};



TEST_F(RiskTests, TestRiskLookbackDef)
{
	auto monthly_trigger_node = StrategyMonthlyRunnerNode::pyMake(
		exchange_sptr
	);
	auto inv_vol_weight_node = std::make_shared< InvVolWeight>(
		60,
		monthly_trigger_node
	);
}