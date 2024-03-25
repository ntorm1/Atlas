#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include "AtlasPointer.hpp"
#include "standard/AtlasCore.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "standard/AtlasEnums.hpp"

namespace Atlas {

//============================================================================
class Measure {
  friend class Tracer;

private:
  virtual void measure(size_t m_idx) noexcept = 0;
  void reset() noexcept;

protected:
  NotNullPtr<Exchange const> m_exchange;
  NotNullPtr<Tracer const> m_tracer;
  TracerType m_type;
  LinAlg::EigenMatrixXd m_values;

public:
  Measure(TracerType type, Exchange const &exchange,
          Tracer const *tracer) noexcept;
  Measure(Measure const &) = delete;
  Measure(Measure &&) = delete;
  Measure &operator=(Measure const &) = delete;
  Measure &operator=(Measure &&) = delete;
  ATLAS_API virtual ~Measure() noexcept;
  auto const &getValues() const noexcept { return m_values; }
  TracerType getType() const noexcept { return m_type; }
};

//============================================================================
class NLVMeasure final : public Measure {
private:
  void measure(size_t m_idx) noexcept override;

public:
  NLVMeasure(Exchange const &exchange, Tracer const *tracer) noexcept;
  ATLAS_API ~NLVMeasure() noexcept;
};

//============================================================================
class WeightMeasure final : public Measure {
private:
  void measure(size_t m_idx) noexcept override;

public:
  WeightMeasure(Exchange const &exchange, Tracer const *tracer) noexcept;
  ATLAS_API ~WeightMeasure() noexcept;
};

//============================================================================
class VolatilityMeasure final : public Measure {
private:
  size_t m_days_per_year = 252;
  void measure(size_t m_idx) noexcept override;
public:
  VolatilityMeasure(Exchange const &exchange, Tracer const *tracer, size_t days_per_year) noexcept;
  ATLAS_API ~VolatilityMeasure() noexcept;
};



} // namespace Atlas