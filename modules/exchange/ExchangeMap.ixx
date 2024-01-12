module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ExchangeMapModule;

import AtlasCore;

namespace Atlas
{

struct ExchangeMapImpl;

export class ExchangeMap
{
	friend class Hydra;
private:
	UniquePtr<ExchangeMapImpl> m_impl;

	void build() noexcept;
	Vector<Int64> const& getTimestamps() const noexcept;

	Result<Exchange*, AtlasException> addExchange(
		String name,
		String source
	) noexcept;

	Result<Exchange*, AtlasException> getExchange(
		String const& name
	) const noexcept;

	void step() noexcept;

public:
	ExchangeMap() noexcept;
	ATLAS_API ~ExchangeMap() noexcept;
	ExchangeMap(const ExchangeMap&) = delete;
	ExchangeMap(ExchangeMap&&) = delete;
	ExchangeMap& operator=(const ExchangeMap&) = delete;
	ExchangeMap& operator=(ExchangeMap&&) = delete;


};

}