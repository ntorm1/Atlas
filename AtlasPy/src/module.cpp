#define  _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
#include <expected>
#include <optional>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

import PortfolioModule;

import AllocationNodeModule;
import AssetNodeModule;
import AtlasEnumsModule;
import AtlasException;
import AtlasCore;
import CommissionsModule;
import ExchangeModule;
import HelperNodesModule;
import HydraModule;
import RankNodeModule;
import ExchangeNodeModule;
import StrategyModule;
import StrategyNodeModule;
import StrategyBufferModule;
import RiskNodeModule;



namespace py = pybind11;



PYBIND11_MODULE(AtlasPy, m) {
    auto m_core = m.def_submodule("core");
    auto m_ast = m.def_submodule("ast");
    

    // ======= PYTHON API ======= //
    py::class_<Atlas::Portfolio, std::shared_ptr<Atlas::Portfolio>>(m_core, "Portfolio")
        .def(
            "getName",
            &Atlas::Portfolio::getName,
            "get unique id of the portfolio"
        );
    py::class_<Atlas::Hydra, std::shared_ptr<Atlas::Hydra>>(m_core, "Hydra")
        .def("build", &Atlas::Hydra::pyBuild)
        .def("run", &Atlas::Hydra::run)
        .def("step", &Atlas::Hydra::step)
        .def("removeStrategy", &Atlas::Hydra::removeStrategy)
        .def("reset", &Atlas::Hydra::pyReset)
        .def("addExchange", &Atlas::Hydra::pyAddExchange)
        .def("getExchange", &Atlas::Hydra::pyGetExchange)
        .def("addStrategy", &Atlas::Hydra::pyAddStrategy,
            py::arg("strategy"),
			py::arg("replace_if_exists") = false
        )
        .def("addPortfolio", &Atlas::Hydra::pyAddPortfolio)
        .def("getPortfolio", &Atlas::Hydra::pyGetPortfolio)
        .def(py::init<>());

    py::class_<Atlas::AST::CovarianceNode, std::shared_ptr<Atlas::AST::CovarianceNode>>(m_ast, "CovarianceNode")
        .def("getCovarianceMatrix", &Atlas::AST::CovarianceNode::getCovariance, py::return_value_policy::reference_internal);

    //py::class_<Atlas::CommisionManager, std::shared_ptr<Atlas::CommisionManager>>(m, "CommisionManager");


    py::class_<Atlas::Exchange, std::shared_ptr<Atlas::Exchange>>(m_core, "Exchange")
        .def("getTimestamps", &Atlas::Exchange::getTimestamps)
        .def("getCovarianceNode", &Atlas::Exchange::getCovarianceNode)
        .def("getAssetMap", &Atlas::Exchange::getAssetMap)
        .def("getName",&Atlas::Exchange::getName,"get unique id of the exchange");

    py::enum_<Atlas::TracerType>(m, "TracerType")
        .value("NLV", Atlas::TracerType::NLV)
        .value("WEIGHTS", Atlas::TracerType::WEIGHTS)
        .export_values();

    // ======= AST API ======= //
    py::enum_<Atlas::AST::ExchangeViewFilterType>(m_ast, "ExchangeViewFilterType")
        .value("GREATER_THAN", Atlas::AST::ExchangeViewFilterType::GREATER_THAN)
        .export_values();
    py::enum_<Atlas::AST::AllocationType>(m_ast, "AllocationType")
        .value("UNIFORM", Atlas::AST::AllocationType::UNIFORM)
        .value("CONDITIONAL_SPLIT", Atlas::AST::AllocationType::CONDITIONAL_SPLIT)
        .export_values();
    py::enum_<Atlas::AST::EVRankType>(m_ast, "EVRankType")
        .value("NLARGEST", Atlas::AST::EVRankType::NLARGEST)
        .value("NSMALLEST", Atlas::AST::EVRankType::NSMALLEST)
        .value("NEXTREME", Atlas::AST::EVRankType::NEXTREME)
        .export_values();


    py::class_<Atlas::AST::AssetReadNode, std::shared_ptr<Atlas::AST::AssetReadNode>>(m_ast, "AssetReadNode")
        .def_static("make", &Atlas::AST::AssetReadNode::pyMake);

    py::class_<Atlas::AST::AssetProductNode, std::shared_ptr<Atlas::AST::AssetProductNode>>(m_ast, "AssetProductNode")
        .def_static("make", &Atlas::AST::AssetProductNode::make);

    py::class_<Atlas::AST::AssetQuotientNode, std::shared_ptr<Atlas::AST::AssetQuotientNode>>(m_ast, "AssetQuotientNode")
        .def_static("make", &Atlas::AST::AssetQuotientNode::make);

    py::class_<Atlas::AST::AssetDifferenceNode, std::shared_ptr<Atlas::AST::AssetDifferenceNode>>(m_ast, "AssetDifferenceNode")
        .def_static("make", &Atlas::AST::AssetDifferenceNode::make);

    py::class_<Atlas::AST::AssetSumNode, std::shared_ptr<Atlas::AST::AssetSumNode>>(m_ast, "AssetSumNode")
        .def_static("make", &Atlas::AST::AssetSumNode::make);

    py::class_<Atlas::AST::AssetOpNodeVariant>(m_ast, "AssetOpNodeVariant")
        .def(py::init<Atlas::AST::AssetOpNodeVariant::node_variant>());

    py::class_<Atlas::AST::ExchangeViewFilter, std::shared_ptr<Atlas::AST::ExchangeViewFilter>>(m_ast, "ExchangeViewFilter")
        .def(py::init<Atlas::AST::ExchangeViewFilterType, double>());

    py::class_<Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::StrategyBufferOpNode>>(m_ast, "StrategyBufferOpNode");

    py::class_<Atlas::AST::ExchangeViewNode, Atlas::AST::StrategyBufferOpNode,  std::shared_ptr<Atlas::AST::ExchangeViewNode>>(m_ast, "ExchangeViewNode")
        .def_static("make", &Atlas::AST::ExchangeViewNode::make,
            py::arg("exchange"),
            py::arg("asset_op_node"),
            py::arg("filter") = std::nullopt);

    py::class_<Atlas::AST::EVRankNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::EVRankNode>>(m_ast, "EVRankNode")
        .def_static("make", &Atlas::AST::EVRankNode::make,
            py::arg("ev"),
            py::arg("type"),
            py::arg("count"),
            py::arg("rank_in_place") = false);


    py::class_<Atlas::AST::TriggerNode, std::shared_ptr<Atlas::AST::TriggerNode>>(m_ast, "TriggerNode")
        .def("getMask", &Atlas::AST::TriggerNode::getMask, py::return_value_policy::reference_internal);

    py::class_<Atlas::AST::StrategyMonthlyRunnerNode, Atlas::AST::TriggerNode, std::shared_ptr<Atlas::AST::StrategyMonthlyRunnerNode>>(m_ast, "StrategyMonthlyRunnerNode")
        .def_static("make", &Atlas::AST::StrategyMonthlyRunnerNode::make,
            py::arg("exchange"),
            py::arg("eom_trigger") = false
        );

    py::class_<Atlas::AST::AllocationBaseNode, std::shared_ptr<Atlas::AST::AllocationBaseNode>>(m_ast, "AllocationBaseNode")
        //.def("setCommissionManager", &Atlas::AST::AllocationBaseNode::AllocationBaseNode)
        .def("setWeightScale", &Atlas::AST::AllocationBaseNode::setWeightScale);

    py::class_<Atlas::AST::FixedAllocationNode, Atlas::AST::AllocationBaseNode, std::shared_ptr<Atlas::AST::FixedAllocationNode>>(m_ast, "FixedAllocationNode")
        .def_static("make", &Atlas::AST::FixedAllocationNode::pyMake);

    py::class_<Atlas::AST::AllocationNode, Atlas::AST::AllocationBaseNode, std::shared_ptr<Atlas::AST::AllocationNode>>(m_ast, "AllocationNode")
        .def_static("make", &Atlas::AST::AllocationNode::pyMake,
            py::arg("exchange_view"),
            py::arg("type") = Atlas::AST::AllocationType::UNIFORM,
            py::arg("alloc_param") = py::none(),
            py::arg("epsilon") = 0.000f
        );

    py::class_<Atlas::AST::AllocationWeightNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AllocationWeightNode>>(m_ast, "AllocationWeightNode");

    py::class_<Atlas::AST::InvVolWeight, Atlas::AST::AllocationWeightNode, std::shared_ptr<Atlas::AST::InvVolWeight>>(m_ast, "InvVolWeight")
        .def(py::init<std::shared_ptr<Atlas::AST::CovarianceNode>, std::optional<double>>());


    py::class_<Atlas::AST::StrategyNode, std::shared_ptr<Atlas::AST::StrategyNode>>(m_ast, "StrategyNode")
        .def_static("make", &Atlas::AST::StrategyNode::make,
            py::arg("allocation"),
            py::arg("portfolio")
        );

    py::class_<Atlas::Strategy, std::shared_ptr<Atlas::Strategy>>(m_core, "Strategy")
        .def("getNLV", &Atlas::Strategy::getNLV)
        .def("getName", &Atlas::Strategy::getName)
        .def("enableTracerHistory", &Atlas::Strategy::enableTracerHistory)
        .def("getHistory", &Atlas::Strategy::getHistory, py::return_value_policy::reference_internal)
        .def("getWeightHistory", &Atlas::Strategy::getWeightHistory, py::return_value_policy::reference_internal)
        .def(py::init<std::string, std::shared_ptr<Atlas::AST::StrategyNode>, double>());

}

int main() {
	return 0;
}
