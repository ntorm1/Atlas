module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module ClusterNodeModule;

import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;
import AtlasLinAlg;

namespace Atlas {

namespace AST {

class ClusterNode;

//============================================================================
export enum ClusterOp { DEMEAN, STANDARDIZE };

//============================================================================
export struct ClusterNodeConfig {
  ClusterOp cluster_op;
  size_t cluster_count;
  size_t max_iterations;
  SharedPtr<TriggerNode> trigger;

  ClusterNodeConfig(ClusterOp cluster_op, size_t cluster_count,
                    size_t max_iterations,
                    SharedPtr<TriggerNode> trigger) noexcept
      : cluster_op(cluster_op), cluster_count(cluster_count), trigger(trigger),
        max_iterations(max_iterations) {}
};

//============================================================================
export class ClusterNode : public StrategyBufferOpNode {
private:
  size_t m_warmup;
  size_t m_last_index;
  LinAlg::EigenMatrixXd m_data;
  LinAlg::EigenVectorXi m_cluster;
  Vector<SharedPtr<AST::StrategyBufferOpNode>> m_features;
  SharedPtr<AST::StrategyBufferOpNode> m_target;
  ClusterNodeConfig m_config;

protected:
public:
  ClusterNode(Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
              SharedPtr<AST::StrategyBufferOpNode> target,
              ClusterNodeConfig config) noexcept;
  ~ClusterNode() noexcept;

  [[nodiscard]] size_t getWarmup() const noexcept override final {
    return m_warmup;
  }

  void evaluate(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override final;
};

} // namespace AST

} // namespace Atlas