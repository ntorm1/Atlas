module;
#include <Eigen/Dense>
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module CommissionsModule;

import AtlasCore;
import AtlasLinAlg;

namespace Atlas
{

class CommissionManagerFactory;

//============================================================================
export class CommisionManager
{
	friend class AllocationBaseNode;
private:
	friend class CommissionManagerFactory;

	Option<double> m_commission_pct = std::nullopt;
	Option<double> m_fixed_commision = std::nullopt;
	Strategy& m_strategy;
	bool m_has_commission = false;

	double calculateFixedCommission(
		Eigen::VectorXd const& target_weights,
		Eigen::VectorXd const& current_weights
	) noexcept;
	double calculatePctCommission(
		double nlv,
		Eigen::VectorXd const& target_weights,
		Eigen::VectorXd const& current_weights
	) noexcept;


	CommisionManager(Strategy& m_strategy) noexcept;
public:
	~CommisionManager() = default;

	bool hasCommission() const noexcept { return m_has_commission; }
	void calculateCommission(
		Eigen::VectorXd const& target_weights,
		Eigen::VectorXd const& current_weights
	) noexcept;
	void setCommissionPct(double commission_pct) { m_commission_pct = commission_pct; m_has_commission = true; }
	void setFixedCommission(double fixed_commission) { m_fixed_commision = fixed_commission; m_has_commission = true; }

};


//============================================================================
export class CommissionManagerFactory
{
	friend class Strategy;

private:
	static SharedPtr<CommisionManager> create(Strategy& strategy) noexcept
	{
		return std::shared_ptr<CommisionManager>(new CommisionManager(strategy));
	}

public:
	CommissionManagerFactory() = default;
	~CommissionManagerFactory() = default;
};




}