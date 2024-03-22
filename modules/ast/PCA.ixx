module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include <mutex>
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
  std::mutex m_mutex;
  String m_id;
  size_t m_last_index;
  size_t m_warmup;
  Vector<SharedPtr<AST::StrategyBufferOpNode>> m_features;
  size_t m_components;
  LinAlg::EigenMatrixXd m_data;
  LinAlg::EigenMatrixXd m_components_data;

public:
  PCAModel(String id, Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
           size_t components) noexcept;
  ~PCAModel() noexcept;

  [[nodiscard]] bool
  isSame(StrategyBufferOpNode const* other) const noexcept override;
  void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
  void build() noexcept;
  void reset() noexcept override;
  [[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
  auto const &getFeatures() const noexcept { return m_features; }
};

} // namespace AST

} // namespace Atlas