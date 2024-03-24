module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include "AtlasPointer.hpp"
export module MeasureModule;

import AtlasCore;
import AtlasLinAlg;
import AtlasEnumsModule;

namespace Atlas {

//============================================================================
export class Measure {
protected:
  NotNullPtr<Exchange const> m_exchange;
  TracerType m_type;
  LinAlg::EigenMatrixXd m_values;


public:
  Measure(TracerType type, Exchange const &exchange) noexcept;
  Measure(Measure const &) = delete;
  Measure(Measure &&) = delete;
  Measure &operator=(Measure const &) = delete;
  Measure &operator=(Measure &&) = delete;
  ATLAS_API ~Measure() noexcept;

};



} // namespace Atlas