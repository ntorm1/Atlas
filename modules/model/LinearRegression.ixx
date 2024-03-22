module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module LinearRegressionModule;

import ModelBaseModule;
import AtlasLinAlg;
import AtlasCore;

namespace Atlas {

namespace Model {

class LinearRegressionModel;

//============================================================================
export enum LinearRegressionSolver { LDLT, ColPivHouseholderQR, Lasso, Ridge };

//============================================================================
export class LinearRegressionModelConfig {
public:
  SharedPtr<ModelConfig> m_base_config;
  LinearRegressionSolver m_solver;
  bool m_fit_intercept = true;
  bool m_orthogonalize_features = false;

  ATLAS_API LinearRegressionModelConfig(
      SharedPtr<ModelConfig> base_config,
      LinearRegressionSolver solver = LinearRegressionSolver::LDLT) noexcept;
  ATLAS_API ~LinearRegressionModelConfig() noexcept = default;
};

//============================================================================
export class LassoRegressionModelConfig : public LinearRegressionModelConfig {
public:
  double m_alpha = 1.0;
  double m_epsilon = 1e-4;
  size_t m_max_iter = 1000;
  ATLAS_API LassoRegressionModelConfig(SharedPtr<ModelConfig> base_config,
                                       double alpha = 1.0,
                                       double epsilon = 1e-4,
                                       size_t max_iter = 1000) noexcept;
  ATLAS_API ~LassoRegressionModelConfig() noexcept = default;
};

//============================================================================
export class LinearRegressionModel : public ModelBase {
private:
  LinAlg::EigenVectorXd m_theta;
  LinAlg::EigenVectorXd m_pvalues;
  SharedPtr<const LinearRegressionModelConfig> m_lr_config;

public:
  ATLAS_API LinearRegressionModel(
      String id, Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
      SharedPtr<ModelTarget> target,
      SharedPtr<const LinearRegressionModelConfig> config) noexcept;
  ATLAS_API ~LinearRegressionModel() noexcept;

  ATLAS_API auto const &getTheta() const noexcept { return m_theta; }
  ATLAS_API auto const &getX() const noexcept { return m_X; }
  ATLAS_API auto const &getY() const noexcept { return m_y; }

  void train() noexcept override;
  void reset() noexcept override;
  void predict() noexcept override;
  [[nodiscard]] bool
  isSame(StrategyBufferOpNode const* other) const noexcept override;
};

} // namespace Model

} // namespace Atlas