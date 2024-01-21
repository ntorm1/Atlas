module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module PortfolioModule;

import AtlasCore;

namespace Atlas
{

class PortfolioImpl;

export class Portfolio
{
private:
	UniquePtr<PortfolioImpl> m_impl;
	String m_name;
	size_t m_id;

public:
	Portfolio(
		String name,
		size_t id,
		Exchange& exchange,
		double initial_cash
	) noexcept;

	ATLAS_API ~Portfolio() noexcept;
	double getInitialCash() const noexcept;
	auto const& getName() const noexcept { return m_name; }
	auto const& getId() const noexcept { return m_id; }
};

}