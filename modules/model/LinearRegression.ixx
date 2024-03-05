module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module LinearRegressionModule;

import ModelBaseModule;
import AtlasLinAlg;
import AtlasCore;

namespace Atlas
{

namespace Model
{

struct LinearRegressionModelImpl;
class LinearRegressionModel;

//============================================================================
export enum LinearRegressionSolver
{
	LDLT,
	ColPivHouseholderQR,
};



//============================================================================
export class LinearRegressionModelConfig
{
	friend class LinearRegressionModel;
private:
	SharedPtr<ModelConfig> m_base_config;
	LinearRegressionSolver m_solver;
	bool m_fit_intercept = true;

public:
	ATLAS_API LinearRegressionModelConfig(
		SharedPtr<ModelConfig> base_config,
		LinearRegressionSolver solver = LinearRegressionSolver::LDLT,
		bool fit_intercept = true
	) noexcept;
	ATLAS_API ~LinearRegressionModelConfig() noexcept = default;
};



//============================================================================
export class LinearRegressionModel : public ModelBase
{
private:
	size_t m_buffer_idx = 0;
	LinearRegressionModelImpl* m_impl;
	LinAlg::EigenMatrixXd m_X;
	LinAlg::EigenVectorXd m_y;
	LinAlg::EigenVectorXd m_theta;
	SharedPtr<const LinearRegressionModelConfig> m_lr_config;

public:
	ATLAS_API LinearRegressionModel(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<ModelTarget> target,
		SharedPtr<const LinearRegressionModelConfig> config
	) noexcept;
	ATLAS_API ~LinearRegressionModel() noexcept;

	ATLAS_API auto const& getTheta() const noexcept  { return m_theta; }
	ATLAS_API auto const& getX() const noexcept { return m_X; }
	ATLAS_API auto const& getY() const noexcept { return m_y; }

	void train() noexcept override;
	void reset() noexcept override;
	void step() noexcept override;
	void predict() noexcept override;
	[[nodiscard]] bool isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept override;
};

}


}