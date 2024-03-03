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
	LinearRegressionModelConfig(
		SharedPtr<ModelConfig> base_config,
		LinearRegressionSolver solver = LinearRegressionSolver::LDLT,
		bool fit_intercept = true
	) noexcept;
};



//============================================================================
export class LinearRegressionModel : public ModelBase
{
private:
	size_t m_buffer_idx = 0;
	LinearRegressionModelImpl* m_impl;
	LinAlg::EigenMatrixXd m_X;
	LinAlg::EigenMatrixXd m_y;
	LinAlg::EigenVectorXd m_theta;
	SharedPtr<const LinearRegressionModelConfig> m_lr_config;

public:
	LinearRegressionModel(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<AST::StrategyBufferOpNode> target,
		SharedPtr<const LinearRegressionModelConfig> config
	) noexcept;
	~LinearRegressionModel() noexcept;

	void train() noexcept override;
	void reset() noexcept override;
	void step() noexcept override;
	void predict() noexcept override;
};

}


}