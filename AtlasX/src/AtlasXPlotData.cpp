

#include "../include/AtlasXPlotData.h"
#include "../include/AtlasXImpl.h"

namespace AtlasX
{


//==============================================================================
AtlasXPlotBuilder::AtlasXPlotBuilder(
	AtlasXAppImpl* app
) noexcept :
	m_app(app)
{
}


//==============================================================================
AtlasXPlotBuilder::~AtlasXPlotBuilder() noexcept
{
}


//==============================================================================
Option<Span<const Int64>>
AtlasXPlotBuilder::getExchangeTimeStamps(
	const String& exchange_name
) noexcept
{
	auto exchange = m_app->getExchange(exchange_name);
	if (!exchange)
	{
		return std::nullopt;
	}
	Vector<Int64> const& vec = m_app->getTimestamps(*exchange);
	return Span<const Int64>(vec);
}


//==============================================================================
Option<Span<const double>>
AtlasXPlotBuilder::getStrategyHistory(const String& strategy_name, const String& history_type) noexcept
{
	return Option<Span<const double>>();
}


//==============================================================================
HashMap<String, size_t> const&
AtlasXPlotBuilder::strategyHistoryTypeMap() noexcept
{
	static HashMap<String, size_t> map = {
		{"NLV", 0},
		{"Volatility", 1}
	};
	return map;
}

}