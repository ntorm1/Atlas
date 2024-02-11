#pragma once

#include "AtlasXTypes.h"


namespace AtlasX
{

class AtlasXAppImpl;


//============================================================================
class AtlasXPlotBuilder {

private:
	AtlasXAppImpl* m_app;

public:
	Option<Span<const Int64>> getStrategyTimeStamps(const String& strategy_name) noexcept;
	Option<Span<const double>> getStrategyHistory(
		const String& strategy_name,
		const String& history_type
	) noexcept;

	static HashMap<String, size_t> const& strategyHistoryTypeMap() noexcept;

	AtlasXPlotBuilder(AtlasXAppImpl* app) noexcept;
	~AtlasXPlotBuilder() noexcept;
};



}