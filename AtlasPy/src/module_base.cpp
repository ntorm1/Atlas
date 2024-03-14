#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
#include "module_base.h"

#include <pybind11/stl.h>
#include <pybind11/eigen.h>

import AtlasEnumsModule;
import CommissionsModule;
import ExchangeModule;
import HydraModule;
import ModelBaseModule;
import PortfolioModule;
import StrategyModule;
import ObserverNodeBaseModule;
import HelperNodesModule;
import RiskNodeModule;



void wrap_base(py::module& m_core)
{
    py::class_<Atlas::Portfolio, std::shared_ptr<Atlas::Portfolio>>(m_core, "Portfolio")
        .def(
            "getName",
            &Atlas::Portfolio::getName,
            "get unique id of the portfolio"
        );
    py::class_<Atlas::Hydra, std::shared_ptr<Atlas::Hydra>>(m_core, "Hydra")
        .def("build", &Atlas::Hydra::pyBuild)
        .def("run", &Atlas::Hydra::pyRun)
        .def("step", &Atlas::Hydra::step)
        .def("removeStrategy", &Atlas::Hydra::removeStrategy)
        .def("reset", &Atlas::Hydra::pyReset)
        .def("addExchange", &Atlas::Hydra::pyAddExchange,
            py::arg("name"),
            py::arg("source"),
            py::arg("datetime_format") = std::nullopt
        )
        .def("getExchange", &Atlas::Hydra::pyGetExchange)
        .def("addStrategy", &Atlas::Hydra::pyAddStrategy,
            py::arg("strategy"),
            py::arg("replace_if_exists") = false
        )
        .def("addPortfolio", &Atlas::Hydra::pyAddPortfolio)
        .def("getPortfolio", &Atlas::Hydra::pyGetPortfolio)
        .def(py::init<>());


    py::class_<Atlas::CommisionManager, std::shared_ptr<Atlas::CommisionManager>>(m_core, "CommisionManager")
        .def("setCommissionPct", &Atlas::CommisionManager::setCommissionPct)
        .def("setFixedCommission", &Atlas::CommisionManager::setFixedCommission);

    py::class_<Atlas::Exchange, std::shared_ptr<Atlas::Exchange>>(m_core, "Exchange")
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
        .def("getName", &Atlas::Exchange::getName, "get unique id of the exchange");


}
