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
	void reset() noexcept;
	void step() noexcept;

	Vector<Int64> const& getTimestamps() const noexcept;

	Result<SharedPtr<Exchange>, AtlasException> addExchange(
		String name,
		String source
	) noexcept;

	Result<Exchange*, AtlasException> getExchange(
		String const& name
	) const noexcept;


public:
	ExchangeMap() noexcept;
	ATLAS_API ~ExchangeMap() noexcept;
	ExchangeMap(const ExchangeMap&) = delete;
	ExchangeMap(ExchangeMap&&) = delete;
	ExchangeMap& operator=(const ExchangeMap&) = delete;
	ExchangeMap& operator=(ExchangeMap&&) = delete;

	ATLAS_API HashMap<String, size_t> const& getExchangeIds() const noexcept;
	Result<Exchange const*, AtlasException> getExchangeConst(
		String const& name
	) const noexcept;


};

}