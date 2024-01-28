#pragma once

#include <QStringList>

#include "../include/AtlasXTypes.h"



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

	//============================================================================
	String const& getEnvPath() const noexcept { return env_path; }

	//============================================================================
	void step() noexcept;

	//============================================================================
	SharedPtr<Atlas::Hydra> getHydra() noexcept { return hydra; }

	//============================================================================
	void run() noexcept;

	//============================================================================
	Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException> addExchange(
		String name,
		String source
	) noexcept;

	//============================================================================
	Result<SharedPtr<Atlas::Portfolio>, Atlas::AtlasException> addPortfolio(
		String name,
		SharedPtr<Atlas::Exchange> exchange,
		double intial_cash
	) noexcept;

	//============================================================================
	Result<SharedPtr<Atlas::Portfolio>, Atlas::AtlasException> getPortfolio(
		String name
	) noexcept;


	//============================================================================
	Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException> getExchange(
		String name
	) noexcept;

	//============================================================================
	HashMap<String, size_t> getPortfolioIdxMap() const noexcept;

	//============================================================================
	Result<bool, Atlas::AtlasException> deserialize(
		String path
	) noexcept;

	//============================================================================
	Result<bool, Atlas::AtlasException> serialize(
		String path
	) noexcept;

	//============================================================================
	Result<bool, Atlas::AtlasException> build() noexcept;

	//============================================================================
	Result<Atlas::LinAlg::EigenConstRowView<double>, Atlas::AtlasException>
	getAssetSlice(String const& asset_name) const noexcept;

	//============================================================================
	HashMap<String, size_t> getExchangeIds() noexcept;

	//============================================================================
	HashMap<String, size_t> const& getExchangeHeaders(SharedPtr<Atlas::Exchange> exchange) noexcept;
	
	//============================================================================
	Vector<Int64> const& getTimestamps(SharedPtr<Atlas::Exchange> exchange) noexcept;

	//============================================================================
	Vector<Int64> const& getTimestamps() noexcept;
	
	//============================================================================
	QStringList const& getTimestampsStr(SharedPtr<Atlas::Exchange> exchange) noexcept;

	//============================================================================
	Int64 currentGlobalTime() const noexcept;

	//============================================================================
	Int64 nextGlobalTime() const noexcept;

	//============================================================================
	size_t getCurrentIdx() const noexcept;

	//============================================================================
	String convertNanosecondsToTime(Int64 nanoseconds);

	//============================================================================
	Option<String> getStrategyParentExchange(
		String const& strategy_name
	) const noexcept;

	//============================================================================
	Atlas::LinAlg::EigenVectorXd const& getStrategyNLV(
		String const& strategy_name
	) const noexcept;

	//============================================================================
	Option<String> getParentExchangeName(
		String const& asset_name
	) const noexcept;

	//============================================================================
	HashMap<String, size_t> getAssetMap(SharedPtr<Atlas::Exchange> e) noexcept;


	
};


}