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
export class Allocator {
  friend class Exchange;
  friend class Hydra;

private:
  UniquePtr<AllocatorImpl> m_impl;
  String m_name;
  size_t m_id = 0;
  void resetBase() noexcept;
  virtual void reset() noexcept = 0;
  virtual void realize() noexcept = 0;
  virtual void load() noexcept = 0;
  void setID(size_t id) noexcept { m_id = id; }
  virtual void step() noexcept = 0;

protected:
  SharedPtr<Tracer> m_tracer;
  LinAlg::EigenVectorXd m_target_weights_buffer;
  bool m_step_call = false;
  Exchange &m_exchange;

public:
  virtual ~Allocator() noexcept;
  ATLAS_API Allocator(String name, Exchange &exchange,
                     Option<SharedPtr<Allocator>>,
            double cash_weight) noexcept;
  LinAlg::EigenVectorXd const &getAllocationBuffer() const noexcept {
    return m_target_weights_buffer;
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