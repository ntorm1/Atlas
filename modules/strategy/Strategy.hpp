#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif

#include "standard/AtlasCore.hpp"
#include "standard/AtlasEnums.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "strategy/Allocator.hpp"

namespace Atlas {

//============================================================================
class StrategyImpl;

//============================================================================
class Strategy : public Allocator {
  friend class AST::StrategyNode;
  friend class AST::StrategyGrid;
  friend class AST::GridDimensionObserver;
  friend class Exchange;
  friend class Hydra;
  friend class CommisionManager;

private:
  UniquePtr<StrategyImpl> m_impl;

  ATLAS_API void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                          target_weights_buffer) noexcept override;
  ATLAS_API void reset() noexcept override;
  ATLAS_API void load() override;
  ATLAS_API void enableCopyWeightsBuffer() noexcept override;
  [[nodiscard]] ATLAS_API size_t getWarmup() const noexcept override;
  [[nodiscard]] SharedPtr<Tracer> getTracerPtr() const noexcept;
  [[nodiscard]] Option<SharedPtr<AST::TradeLimitNode>>
  getTradeLimitNode() const noexcept;
  [[nodiscard]] LinAlg::EigenRef<LinAlg::EigenVectorXd> getPnL() noexcept;
  [[nodiscard]] size_t refreshWarmup() noexcept;
  void setNlv(double nlv_new) noexcept;
  void setTracer(SharedPtr<Tracer> tracer) noexcept;

public:
  ATLAS_API Strategy(String name, SharedPtr<Exchange> exchange,
                     SharedPtr<Allocator> parent,
                     double portfolio_weight) noexcept;
  ATLAS_API virtual SharedPtr<AST::StrategyNode> loadAST() noexcept = 0;
  ATLAS_API ~Strategy() noexcept;

  ATLAS_API const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;

  [[nodiscard]] ATLAS_API Option<SharedPtr<AST::StrategyGrid>>
  getGrid() const noexcept;
  [[nodiscard]] ATLAS_API SharedPtr<CommisionManager>
  initCommissionManager() noexcept;

  ATLAS_API
  [[nodiscard]] Result<SharedPtr<AST::StrategyGrid const>, AtlasException>
  setGridDimmensions(
      std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>>
          dimensions,
      Option<GridType> grid_type = std::nullopt) noexcept;
  ATLAS_API [[nodiscard]] SharedPtr<AST::StrategyGrid const>
  pySetGridDimmensions(
      std::pair<SharedPtr<AST::GridDimension>, SharedPtr<AST::GridDimension>>
          dimensions,
      Option<GridType> grid_type = std::nullopt);
};

} // namespace Atlas
