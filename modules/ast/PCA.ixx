module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module PCAModule;

import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;
import AtlasLinAlg;

namespace Atlas {

namespace AST {

class PCAModel;

//============================================================================
export class PCAModel : public StrategyBufferOpNode {
private:
  String m_id;
  Vector<SharedPtr<AST::StrategyBufferOpNode>> m_features;
  size_t m_components;
  SharedPtr<CovarianceNodeBase> m_cov = nullptr;
  LinAlg::EigenMatrixXd m_data;
  LinAlg::EigenMatrixXd m_components_data;

public:
  PCAModel(String id, Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
           SharedPtr<CovarianceNodeBase> cov, size_t components) noexcept;
  ~PCAModel() noexcept;

  void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;

  void reset() noexcept override;
};

} // namespace AST

} // namespace Atlas