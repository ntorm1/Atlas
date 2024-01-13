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
	ATLAS_API Result<Exchange*, AtlasException> addExchange(
		String name,
		String source
	) noexcept;

	//============================================================================
	ATLAS_API Result<Exchange*, AtlasException> getExchange(
		String const& name
	) const noexcept;

	//============================================================================
	ATLAS_API Result<Strategy const*, AtlasException> addStrategy(
		UniquePtr<Strategy> strategy
	) noexcept;

	//============================================================================
	ATLAS_API Result<Portfolio*, AtlasException> addPortfolio(
		String name,
		Exchange& exchange,
		double initial_cash
	) noexcept;

	//============================================================================
	ATLAS_API Result<bool, AtlasException> build();

	//============================================================================
	ATLAS_API void step() noexcept;

	//============================================================================
	ATLAS_API void run() noexcept;

};


}