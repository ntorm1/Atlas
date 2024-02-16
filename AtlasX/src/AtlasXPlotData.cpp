

#include "../include/AtlasXPlotData.h"
#include "../include/AtlasXImpl.h"

namespace AtlasX
{


//==============================================================================
AtlasXStrategyPlotBuilder::AtlasXStrategyPlotBuilder(
	AtlasXAppImpl* app
) noexcept :
	m_app(app)
{
}


//==============================================================================
AtlasXStrategyPlotBuilder::~AtlasXStrategyPlotBuilder() noexcept
{
}


//==============================================================================
Option<Span<const Int64>>
AtlasXStrategyPlotBuilder::getStrategyTimeStamps(
	const String& strategy_name
) noexcept
{
	auto strategy = m_app->getStrategy(strategy_name);
	if (!strategy)
	{
		return std::nullopt;
	}

	auto exchange_name = m_app->getParentExchangeName(*strategy);
	if (!exchange_name)
	{
		return std::nullopt;
	}
	auto exchange = m_app->getExchange(*exchange_name);
	Vector<Int64> const& vec = m_app->getTimestamps(*exchange);
	return Span<const Int64>(vec);
}


//==============================================================================
Option<Span<const double>>
AtlasXStrategyPlotBuilder::getStrategyHistory(
	const String& strategy_name,
	const String& history_type
) noexcept
{
	if (!strategyHistoryTypeMap().contains(history_type))
	{
		return std::nullopt;
	}
	if (history_type == "NLV")
	{
		Atlas::LinAlg::EigenVectorXd const& nlv = m_app->getStrategyMeasure(
			strategy_name,
			"NLV"
		);
		return Span<const double>(const_cast<double*>(nlv.data()), nlv.size());
	}
	if (history_type == "Volatility")
	{
		Atlas::LinAlg::EigenVectorXd const& nlv = m_app->getStrategyMeasure(
			strategy_name,
			"Volatility"
		);
		return Span<const double>(const_cast<double*>(nlv.data()), nlv.size());
	}
	return std::nullopt;
}


//==============================================================================
HashMap<String, size_t> const&
AtlasXStrategyPlotBuilder::strategyHistoryTypeMap() noexcept
{
	static HashMap<String, size_t> map = {
		{"NLV", 0},
		{"Volatility", 1}
	};
	return map;
}


//==============================================================================
AtlasXAssetPlotBuilder::AtlasXAssetPlotBuilder(
	AtlasXAppImpl* app
) noexcept:
	m_app(app)
{
}


//==============================================================================
AtlasXAssetPlotBuilder::~AtlasXAssetPlotBuilder() noexcept
{
}

}