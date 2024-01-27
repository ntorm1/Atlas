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




	// ======= PUBLIC API ======= //
	//============================================================================
	ATLAS_API Result<SharedPtr<Exchange>, AtlasException> addExchange(
		String name,
		String source
	) noexcept;

	//============================================================================
	ATLAS_API Result<SharedPtr<Exchange>, AtlasException> getExchange(
		String const& name
	) const noexcept;

	//============================================================================
	ATLAS_API Result<Strategy const*, AtlasException> addStrategy(
		SharedPtr<Strategy> strategy,
		bool replace_if_exists = false
	) noexcept;

	//============================================================================
	ATLAS_API Result<SharedPtr<Portfolio>, AtlasException> addPortfolio(
		String name,
		Exchange& exchange,
		double initial_cash
	) noexcept;

	//============================================================================
	ATLAS_API ExchangeMap const& getExchangeMap() const noexcept;

	//============================================================================
	ATLAS_API Result<bool, AtlasException> build();

	//============================================================================
	ATLAS_API void removeStrategy(String const& name) noexcept;

	//============================================================================
	ATLAS_API void step() noexcept;

	//============================================================================
	ATLAS_API void run() noexcept;

	//============================================================================
	ATLAS_API Result<bool, AtlasException> reset() noexcept;

	// ======= PYTHON API ======= //
	//============================================================================
	ATLAS_API SharedPtr<Exchange> pyAddExchange(
		String name,
		String source
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
	ATLAS_API void pyBuild();

	//============================================================================
	ATLAS_API void pyReset();
};


}