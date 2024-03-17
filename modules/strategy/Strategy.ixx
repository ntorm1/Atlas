module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module StrategyModule;

import AtlasCore;
import AtlasEnumsModule;
import AtlasAllocatorModule;
import AtlasLinAlg;

namespace Atlas {

//============================================================================
class StrategyImpl;

//============================================================================
export class Strategy : public Allocator {
  friend class AST::StrategyNode;
  friend class AST::StrategyGrid;
  friend class AST::GridDimensionObserver;
  friend class Exchange;
  friend class Hydra;
  friend class CommisionManager;

private:
  double m_portfolio_weight = 0.0;
  UniquePtr<StrategyImpl> m_impl;

  void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> const
                    &target_weights_buffer) noexcept;

  ATLAS_API void
  step(LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;
  ATLAS_API void reset() noexcept override;
  ATLAS_API void load() noexcept override;
  void setNlv(double nlv_new) noexcept;
  void setTracer(SharedPtr<Tracer> tracer) noexcept;
  ATLAS_API [[nodiscard]] size_t getWarmup() const noexcept override;
  [[nodiscard]] SharedPtr<Tracer> getTracerPtr() const noexcept;
  [[nodiscard]] Option<SharedPtr<AST::TradeLimitNode>>
  getTradeLimitNode() const noexcept;
  [[nodiscard]] LinAlg::EigenRef<LinAlg::EigenVectorXd> getPnL() noexcept;
  [[nodiscard]] size_t refreshWarmup() noexcept;
  
public:
  ATLAS_API Strategy(String name, SharedPtr<Exchange> exchange,
                     SharedPtr<Allocator> parent,
                     double portfolio_weight) noexcept;
  ATLAS_API virtual SharedPtr<AST::StrategyNode> loadAST() noexcept = 0;
  ATLAS_API ~Strategy() noexcept;

  ATLAS_API const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;

  ATLAS_API [[nodiscard]] SharedPtr<CommisionManager>
  initCommissionManager() noexcept;
  ATLAS_API [[nodiscard]] Result<bool, AtlasException>
  enableTracerHistory(TracerType t) noexcept;
  ATLAS_API [[nodiscard]] Option<SharedPtr<AST::StrategyGrid>>
  getGrid() const noexcept;
  ATLAS_API void pyEnableTracerHistory(TracerType t);
  ATLAS_API void setVolTracer(SharedPtr<AST::CovarianceNodeBase> node) noexcept;

  ATLAS_API
      [[nodiscard]] Result<SharedPtr<AST::StrategyGrid const>, AtlasException>
      setGridDimmensions(std::pair<SharedPtr<AST::GridDimension>,
                                   SharedPtr<AST::GridDimension>>
                             dimensions,
                         Option<GridType> grid_type = std::nullopt) noexcept;
  ATLAS_API [[nodiscard]] SharedPtr<AST::StrategyGrid const>
  pySetGridDimmensions(
      std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>>
          dimensions,
      Option<GridType> grid_type = std::nullopt);
};

} // namespace Atlas
