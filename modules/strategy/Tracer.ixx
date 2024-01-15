module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module TracerModule;

import AtlasCore;

namespace Atlas
{

//============================================================================
export enum class TracerItem: uint8_t
{
	NLV = 1 << 0,
	CASH = 1 << 1,
	UNITS = 1 << 2,
	ALLOC_PCT = 1 << 3,
};

export constexpr size_t ALLOC_PCT_INDEX = 0;

//============================================================================
export class Tracer
{
	friend class Strategy;
private:
	Eigen::MatrixXd m_data;
	uint8_t  m_flags = 0;
	double m_cash;
	double m_initial_cash;
	double m_nlv;

	bool isTracerFlagSet(TracerItem flag) const noexcept;
	void clearTracerFlag(TracerItem flag) noexcept;
	void setTracerFlag(TracerItem flag) noexcept;
	size_t getTracerCount() const noexcept;

	void evaluate(bool is_reprice) noexcept;
	void reset() noexcept;

	Eigen::MatrixXd& getTracerData() noexcept { return m_data; }

	void setNLV(double nlv) noexcept { m_nlv = nlv; }

public:
	Tracer(Exchange const& exchange, double cash) noexcept;
	ATLAS_API double getCash() const noexcept { return m_cash; }
	ATLAS_API double getNLV() const noexcept { return m_nlv; }
	ATLAS_API double getInitialCash() const noexcept { return m_initial_cash; }
	ATLAS_API double getAllocation(size_t asset_index) const noexcept { return m_data(asset_index, ALLOC_PCT_INDEX); }
};



}