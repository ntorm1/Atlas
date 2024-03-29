#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
#include "module_base.h"

#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include "hydra/Commissions.hpp"
#include "strategy/Allocator.hpp"
#include "strategy/MetaStrategy.hpp"
#include "model/ModelBase.hpp"
#include "hydra/Hydra.hpp"
#include "exchange/Exchange.hpp"
#include "ast/RiskNode.hpp"
#include "ast/ObserverNodeBase.hpp"
#include "ast/HelperNodes.hpp"

void wrap_base(py::module &m_core) {
  py::class_<Atlas::Hydra, std::shared_ptr<Atlas::Hydra>>(m_core, "Hydra")
      .def("build", &Atlas::Hydra::pyBuild)
      .def("run", &Atlas::Hydra::pyRun)
      .def("step", &Atlas::Hydra::step)
      .def("removeStrategy", &Atlas::Hydra::removeStrategy)
      .def("reset", &Atlas::Hydra::pyReset)
      .def("addExchange", &Atlas::Hydra::pyAddExchange, py::arg("name"),
           py::arg("source"), py::arg("datetime_format") = std::nullopt)
      .def("getExchange", &Atlas::Hydra::pyGetExchange)
      .def("getStrategy", &Atlas::Hydra::getStrategy)
      .def("addStrategy", &Atlas::Hydra::pyAddStrategy, py::arg("strategy"),
           py::arg("replace_if_exists") = false)
      .def(py::init<>());

  py::class_<Atlas::CommisionManager, std::shared_ptr<Atlas::CommisionManager>>(
      m_core, "CommisionManager")
      .def("setCommissionPct", &Atlas::CommisionManager::setCommissionPct)
      .def("setFixedCommission", &Atlas::CommisionManager::setFixedCommission);

  py::class_<Atlas::Exchange, std::shared_ptr<Atlas::Exchange>>(m_core,
                                                                "Exchange")
      .def("registerModel", &Atlas::Exchange::registerModel)
      .def("registerObserver", &Atlas::Exchange::registerObserver)
      .def("getObserver", &Atlas::Exchange::getObserver)
      .def("enableNodeCache", &Atlas::Exchange::enableNodeCache)
      .def("getTimestamps", &Atlas::Exchange::getTimestamps)
      .def("getCovarianceNode", &Atlas::Exchange::getCovarianceNode)
      .def("getMarketReturns", &Atlas::Exchange::getMarketReturns,
           py::arg("row_offset") = 0,
           py::return_value_policy::reference_internal)
      .def("getAssetMap", &Atlas::Exchange::getAssetMap)
      .def("getAssetIndex", &Atlas::Exchange::getAssetIndex)
      .def("getCurrentTimestamp", &Atlas::Exchange::getCurrentTimestamp)
      .def("getName", &Atlas::Exchange::getName,
           "get unique id of the exchange");
}
