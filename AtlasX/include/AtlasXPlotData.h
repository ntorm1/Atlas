#pragma once

#include "AtlasXTypes.h"

namespace AtlasX
{

class AtlasXAppImpl;


//============================================================================
class AtlasXStrategyPlotBuilder {

private:
	AtlasXAppImpl* m_app;

public:
	Option<Span<const Int64>> getStrategyTimeStamps(const String& strategy_name) noexcept;
	Option<Span<const double>> getStrategyHistory(
		const String& strategy_name,
		const String& history_type
	) noexcept;

	static HashMap<String, size_t> const& strategyHistoryTypeMap() noexcept;

	AtlasXStrategyPlotBuilder(AtlasXAppImpl* app) noexcept;
	~AtlasXStrategyPlotBuilder() noexcept;
};


//============================================================================
class AtlasXAssetPlotBuilder {
private:
	AtlasXAppImpl* m_app;

public:

	AtlasXAssetPlotBuilder(AtlasXAppImpl* app) noexcept;
	~AtlasXAssetPlotBuilder() noexcept;

	HashMap<String, size_t> const& getExchangeHeaders(SharedPtr<Atlas::Exchange> exchange) noexcept;
	Option<SharedPtr<Atlas::Exchange>> getParentExchange(String const& asset_name) noexcept;
	Vector<Int64> const& getTimestamps(SharedPtr<Atlas::Exchange> exchange) noexcept;
	Option<Vector<double>> getAssetSlice(
		SharedPtr<Atlas::Exchange> exchange,
		String const& asset_name
	) const noexcept;
};


}