#pragma once

#include <QStringList>

#include "AtlasXTypes.h"
#include "AtlasInter.h"


import AtlasException;
import AtlasLinAlg;

namespace AtlasX
{

struct AtlasXAppImpl
{
	friend class AtlasXApp;
private:
	SharedPtr<Atlas::Hydra> hydra;
	String env_path = "";
	String env_name = "";
	HashMap<String, QStringList> timestamp_cache;
	AtlasXExchangeManager* exchange_manager;
	AtlasXStrategyManager* strategy_manager;

public:
	AtlasXAppImpl();
	~AtlasXAppImpl();

	
	String const& getEnvPath() const noexcept { return env_path; }

	
	void step() noexcept;
	[[nodiscard]] Result<bool, Atlas::AtlasException> run() noexcept;
	[[nodiscard]] Result<bool, Atlas::AtlasException> build() noexcept;
	[[nodiscard]] Result<bool, Atlas::AtlasException> reset() noexcept;

	
	SharedPtr<Atlas::Hydra> getHydra() noexcept { return hydra; }
	[[nodiscard]] Result<SharedPtr<Atlas::Portfolio>, Atlas::AtlasException> getPortfolio(
		String name
	) noexcept;
	Option<SharedPtr<Atlas::Strategy>> getStrategy(
		String const& strategy_name
	) noexcept;
	[[nodiscard]] Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException> getExchange(
		String name
	) noexcept;
	[[nodiscard]] Result<bool, Atlas::AtlasException> removeExchange(
		String name
	) noexcept;
	[[nodiscard]]  Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException> addExchange(
		String name,
		String source,
		Option<String> datetime_format
	) noexcept;
	[[nodiscard]]  Result<SharedPtr<Atlas::Portfolio>, Atlas::AtlasException> addPortfolio(
		String name,
		SharedPtr<Atlas::Exchange> exchange,
		double intial_cash
	) noexcept;

	
	[[nodiscard]] Result<bool, Atlas::AtlasException> deserialize(
		String path
	) noexcept;
	[[nodiscard]] Result<bool, Atlas::AtlasException> serialize(
		String path
	) noexcept;

	
	[[nodiscard]] Result<Atlas::LinAlg::EigenConstRowView<double>, Atlas::AtlasException> getAssetSlice(String const& asset_name) const noexcept;
	[[nodiscard]] Option<Vector<double>> getCacheSlice(SharedPtr<Atlas::Exchange> exchange, String const& asset_name, SharedPtr<Atlas::AST::StrategyBufferOpNode> node) noexcept;
	
	HashMap<String, size_t> getExchangeIds() noexcept;
	size_t* getCurrentIdxPtr() const noexcept;
	Vector<Int64> const& getTimestamps(SharedPtr<Atlas::Exchange> exchange) noexcept;
	Vector<Int64> const& getTimestamps() noexcept;
	QStringList const& getTimestampsStr(SharedPtr<Atlas::Exchange> exchange) noexcept;
	Int64 currentGlobalTime() const noexcept;
	Int64 nextGlobalTime() const noexcept;
	size_t getCurrentIdx() const noexcept;
	String convertNanosecondsToTime(Int64 nanoseconds);


	[[nodiscard]] Option<String> getStrategyParentExchange(
		String const& strategy_name
	) const noexcept;
	Atlas::LinAlg::EigenVectorXd const& getStrategyMeasure(
		String const& strategy_name,
		String const& measure
	) const noexcept;
	[[nodiscard]] Option<String> getParentExchangeName(
		String const& asset_name
	) const noexcept;
	[[nodiscard]] Option<String> getParentExchangeName(
		SharedPtr<Atlas::Strategy> strategy
	) const noexcept;
	HashMap<String, size_t> getPortfolioIdxMap() const noexcept;
	HashMap<String, SharedPtr<Atlas::AST::StrategyBufferOpNode>> getASTCache(SharedPtr<Atlas::Exchange> exchange) const noexcept;
	HashMap<String, size_t> const& getExchangeHeaders(SharedPtr<Atlas::Exchange> exchange) noexcept;
	HashMap<String, size_t> getAssetMap(SharedPtr<Atlas::Exchange> e) noexcept;
	Option<SharedPtr<GridState>> getStrategyGridState(String const& strategy_name) noexcept;
	
};


}