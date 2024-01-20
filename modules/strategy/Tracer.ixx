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
import AtlasEnumsModule;

namespace Atlas
{

//============================================================================
export constexpr size_t ALLOC_PCT_INDEX = 0;


//============================================================================
export class Tracer
{
	friend class Strategy;
private:
	Exchange const& m_exchange;
	Eigen::MatrixXd m_data;
	Eigen::VectorXd m_nlv_history;
	size_t m_idx = 0;
	double m_cash;
	double m_initial_cash;
	double m_nlv;

	void evaluate() noexcept;
	void reset() noexcept;

	Eigen::MatrixXd& getTracerData() noexcept { return m_data; }

	void enableTracerHistory(TracerType t) noexcept;
	void setNLV(double nlv) noexcept { m_nlv = nlv; }
	Eigen::VectorXd const& getHistory(TracerType t) const noexcept;

public:
	Tracer(Exchange const& exchange, double cash) noexcept;
	ATLAS_API double getCash() const noexcept { return m_cash; }
	ATLAS_API double getNLV() const noexcept { return m_nlv; }
	ATLAS_API double getInitialCash() const noexcept { return m_initial_cash; }
	ATLAS_API double getAllocation(size_t asset_index) const noexcept { return m_data(asset_index, ALLOC_PCT_INDEX); }
};



}