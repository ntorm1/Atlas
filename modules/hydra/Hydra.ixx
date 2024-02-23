module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module HydraModule;

import AtlasCore;

namespace Atlas
{

struct HydraImpl;

export enum class HydraState
{
	INIT = 0,
	BUILT = 1,
	RUNING = 2,
	FINISHED = 3
};

export class Hydra
{
private:
	UniquePtr<HydraImpl> m_impl;
	HydraState m_state = HydraState::INIT;

public:
	ATLAS_API Hydra() noexcept;
	ATLAS_API ~Hydra() noexcept;


	ATLAS_API Result<SharedPtr<Exchange>, AtlasException> addExchange(
		String name,
		String source,
		Option<String> datetime_format = std::nullopt
	) noexcept;
	ATLAS_API Result<SharedPtr<Exchange>, AtlasException> getExchange(
		String const& name
	) const noexcept;
	ATLAS_API Result<Strategy const*, AtlasException> addStrategy(
		SharedPtr<Strategy> strategy,
		bool replace_if_exists = false
	) noexcept;
	ATLAS_API Option<SharedPtr<Strategy>> getStrategy(String const& strategy_name) noexcept;
	ATLAS_API void removeStrategy(String const& name) noexcept;
	ATLAS_API Result<SharedPtr<Portfolio>, AtlasException> addPortfolio(
		String name,
		Exchange& exchange,
		double initial_cash
	) noexcept;
	ATLAS_API Result<bool, AtlasException> removeExchange(String const& name) noexcept;
	ATLAS_API Result<bool, AtlasException> build();
	ATLAS_API void step() noexcept;
	ATLAS_API [[nodiscard]] Result<bool, AtlasException> run() noexcept;
	ATLAS_API Result<bool, AtlasException> reset() noexcept;


	ATLAS_API Int64 currentGlobalTime() const noexcept;
	ATLAS_API Int64 nextGlobalTime() const noexcept;
	ATLAS_API size_t getCurrentIdx() const noexcept;
	ATLAS_API size_t* getCurrentIdxPtr() const noexcept;
	ATLAS_API Vector<Int64> const& getTimestamps() const noexcept;
	ATLAS_API HashMap<String, size_t> getStrategyIdxMap() const noexcept;
	ATLAS_API HashMap<String, size_t> getPortfolioIdxMap() const noexcept;
	ATLAS_API ExchangeMap const& getExchangeMap() const noexcept;
	ATLAS_API Option<String> getParentExchangeName(
		String const& asset_name
	) const noexcept;


	// ======= PYTHON API ======= //
	//============================================================================
	ATLAS_API SharedPtr<Exchange> pyAddExchange(
		String name,
		String source,
		Option<String> datetime_format = std::nullopt
	);

	//============================================================================
	ATLAS_API SharedPtr<Portfolio> pyAddPortfolio(
		String name,
		SharedPtr<Exchange> exchange,
		double initial_cash
	);

	//============================================================================
	ATLAS_API SharedPtr<Strategy> pyAddStrategy(
		SharedPtr<Strategy> strategy,
		bool replace_if_exists = false
	);

	//============================================================================
	ATLAS_API SharedPtr<Exchange> pyGetExchange(
		String const& name
	) const;

	//============================================================================
	ATLAS_API SharedPtr<Portfolio> pyGetPortfolio(
		String const& name
	) const;

	ATLAS_API void pyRun(); 
	ATLAS_API void pyBuild();
	ATLAS_API void pyReset();
};


}