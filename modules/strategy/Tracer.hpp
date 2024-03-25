#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include "AtlasPointer.hpp"
#include "AtlasStruct.hpp"
#include "standard/AtlasCore.hpp"
#include "standard/AtlasEnums.hpp"
#include "standard/AtlasLinAlg.hpp"

namespace Atlas {

//============================================================================
constexpr size_t ALLOC_PCT_INDEX = 0;

//============================================================================
struct StructTracer {
  Tracer const &m_tracer;
  Exchange const &m_exchange;
  Vector<Order> m_orders;
  Vector<Trade> m_trades;
  size_t close_index = 0;
  bool orders_eager = false;

  StructTracer(Exchange const &exchange, Tracer const &tracer) noexcept;
  StructTracer(StructTracer const &) = delete;
  StructTracer(StructTracer &&) = delete;
  StructTracer &operator=(StructTracer const &) = delete;
  StructTracer &operator=(StructTracer &&) = delete;
  ATLAS_API ~StructTracer() noexcept;

  bool eager() const noexcept { return orders_eager; }
  void enabelTracerHistory(TracerType t) noexcept;
  void evaluate(LinAlg::EigenVectorXd const &weights,
                LinAlg::EigenVectorXd const &previous_weights) noexcept;
  void reset() noexcept;
};

//============================================================================
class Tracer {
  friend struct StructTracer;
  friend class Strategy;
  friend class Allocator;
  friend class AST::AllocationBaseNode;
  friend class AST::StrategyGrid;
  friend class Measure;

private:
  Exchange const &m_exchange;
  Allocator const *m_allocator;
  Option<UniquePtr<StructTracer>> m_struct_tracer = std::nullopt;
  Option<SharedPtr<AST::CovarianceNodeBase>> m_covariance;
  Vector<SharedPtr<Measure>> m_measures;
  LinAlg::EigenVectorXd m_pnl;
  LinAlg::EigenVectorXd m_weights_buffer;
  size_t m_idx = 0;
  double m_cash = 0.0;
  double m_initial_cash = 0.0;
  double m_nlv = 0.0;

  void realize() noexcept;
  void evaluate() noexcept;
  void reset() noexcept;

  [[nodiscard]] Result<bool, AtlasException>
  enableTracerHistory(TracerType t) noexcept;
  void setNLV(double nlv) noexcept { m_nlv = nlv; }
  void
  setCovarianceNode(SharedPtr<AST::CovarianceNodeBase> covariance) noexcept {
    m_covariance = covariance;
  }
  [[nodiscard]] LinAlg::EigenVectorXd &getPnL() noexcept;
  void setPnL(LinAlg::EigenRef<LinAlg::EigenVectorXd> pnl) noexcept;
  void initPnL() noexcept;

public:
  Tracer(Allocator const *allocator, Exchange const &exchange,
         double cash) noexcept;
  [[nodiscard]] ATLAS_API Vector<Order> const &getOrders() const noexcept;
  [[nodiscard]] ATLAS_API Option<SharedPtr<Measure>>
  getMeasure(TracerType t) const noexcept;
  ATLAS_API double getCash() const noexcept { return m_cash; }
  ATLAS_API double getNLV() const noexcept { return m_nlv; }
  ATLAS_API double getInitialCash() const noexcept { return m_initial_cash; }

  [[nodiscard]] Exchange const &getExchange() const noexcept {
    return m_exchange;
  }
  [[nodiscard]] auto const &getCovariance() const noexcept {
    return m_covariance;
  }
  [[nodiscard]] NotNullPtr<Allocator const> getAllocator() const noexcept {
    return m_allocator;
  }
};

} // namespace Atlas