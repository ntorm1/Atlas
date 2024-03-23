module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module AtlasAllocatorModule;

import AtlasCore;
import AtlasEnumsModule;
import AtlasLinAlg;

namespace Atlas {

//============================================================================
struct AllocatorImpl;

//============================================================================
struct AllocatorConfig {
  bool can_short = true;
  bool disable_on_breach = false;
  WeightScaleType weight_scale = WeightScaleType::NO_SCALE;
  Option<double> vol_target = std::nullopt;
  Option<double> weight_clip = std::nullopt;
  Option<double> vol_limit = std::nullopt;
  Option<double> risk_contrib_limit = std::nullopt;

  AllocatorConfig() noexcept = default;
  ~AllocatorConfig() noexcept = default;
};

//============================================================================
export class Allocator {
  friend class Exchange;
  friend class Hydra;

private:
  UniquePtr<AllocatorImpl> m_impl;
  String m_name;
  size_t m_id = 0;
  bool m_is_meta;
  void resetBase() noexcept;
  void realize() noexcept;
  void disable(String const& exception) noexcept;

protected:
  SharedPtr<Tracer> m_tracer;
  bool m_step_call = false;
  double m_portfolio_weight = 0.0;
  Exchange &m_exchange;

  [[nodiscard]] size_t getAssetCount() const noexcept;
  void lateRebalance(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;
  void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> const
                    &target_weights_buffer) noexcept;
  void validate(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;


public:
  virtual ~Allocator() noexcept;
  ATLAS_API Allocator(String name, Exchange &exchange,
                      Option<SharedPtr<Allocator>>,
                      double cash_weight) noexcept;
  virtual const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept = 0;
  void setIsMeta(bool is_meta) noexcept { m_is_meta = is_meta; }
  [[nodiscard]] bool getIsMeta() const noexcept { return m_is_meta; }
  virtual void reset() noexcept = 0;
  virtual void load() noexcept = 0;
  virtual void enableCopyWeightsBuffer() noexcept = 0;
  void setID(size_t id) noexcept { m_id = id; }
  void stepBase(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;
  virtual void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                        target_weights_buffer) noexcept = 0;
  [[nodiscard]] Option<SharedPtr<Allocator>> getParent() const noexcept;
  [[nodiscard]] virtual size_t getWarmup() const noexcept = 0;

  ATLAS_API [[nodiscard]] Result<bool, AtlasException>
  enableTracerHistory(TracerType t) noexcept;
  ATLAS_API void pyEnableTracerHistory(TracerType t);
  ATLAS_API void setVolTracer(SharedPtr<AST::CovarianceNodeBase> node) noexcept;
  ATLAS_API [[nodiscard]] double getAllocation() const noexcept {
    return m_portfolio_weight;
  }
  ATLAS_API [[nodiscard]] LinAlg::EigenVectorXd const &
  getHistory(TracerType t) const noexcept;
  ATLAS_API [[nodiscard]] LinAlg::EigenMatrixXd const &
  getWeightHistory() const noexcept;
  ATLAS_API [[nodiscard]] double
  getAllocation(size_t asset_index) const noexcept;
  ATLAS_API [[nodiscard]] auto const &getId() const noexcept { return m_id; }
  ATLAS_API [[nodiscard]] Exchange const &getExchange() const noexcept;
  ATLAS_API [[nodiscard]] Tracer const &getTracer() const noexcept;
  ATLAS_API [[nodiscard]] double getNLV() const noexcept;
  ATLAS_API [[nodiscard]] auto const &getName() const noexcept {
    return m_name;
  }
};

} // namespace Atlas