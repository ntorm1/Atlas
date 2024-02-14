#define  _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
#include <expected>
#include <optional>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

import PortfolioModule;

import AllocationNodeModule;
import AssetNodeModule;
import AtlasEnumsModule;
import AtlasException;
import AtlasLinAlg;
import AtlasCore;
import BaseNodeModule;
import CommissionsModule;
import ExchangeModule;
import ExchangeNodeModule;
import HelperNodesModule;
import HydraModule;
import OptimizeNodeModule;
import RankNodeModule;
import StrategyModule;
import StrategyNodeModule;
import StrategyBufferModule;
import RiskNodeModule;
import TradeNodeModule;



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

    py::class_<Atlas::AST::CovarianceNodeBase, std::shared_ptr<Atlas::AST::CovarianceNodeBase>>(m_ast, "CovarianceNodeBase")
        .def("getCovarianceMatrix", &Atlas::AST::CovarianceNodeBase::getCovariance, py::return_value_policy::reference_internal);

    py::class_<Atlas::AST::CovarianceNode, Atlas::AST::CovarianceNodeBase, std::shared_ptr<Atlas::AST::CovarianceNode>>(m_ast, "CovarianceNode");

    py::class_<Atlas::AST::IncrementalCovarianceNode, Atlas::AST::CovarianceNodeBase, std::shared_ptr<Atlas::AST::IncrementalCovarianceNode>>(m_ast, "IncrementalCovarianceNode");

    py::class_<Atlas::CommisionManager, std::shared_ptr<Atlas::CommisionManager>>(m, "CommisionManager")
        .def("setCommissionPct", &Atlas::CommisionManager::setCommissionPct)
        .def("setFixedCommission", &Atlas::CommisionManager::setFixedCommission);

    py::enum_<Atlas::CovarianceType>(m_ast, "CovarianceType")
        .value("FULL", Atlas::CovarianceType::FULL)
        .value("INCREMENTAL", Atlas::CovarianceType::INCREMENTAL)
        .export_values();

    py::class_<Atlas::Exchange, std::shared_ptr<Atlas::Exchange>>(m_core, "Exchange")
        .def("getTimestamps", &Atlas::Exchange::getTimestamps)
        .def("getCovarianceNode", &Atlas::Exchange::getCovarianceNode)
        .def("getMarketReturns", &Atlas::Exchange::getMarketReturns,
            py::arg("row_offset") = 0,
            py::return_value_policy::reference_internal)
        .def("getAssetMap", &Atlas::Exchange::getAssetMap)
        .def("getAssetIndex", &Atlas::Exchange::getAssetIndex)
        .def("getCurrentTimestamp", &Atlas::Exchange::getCurrentTimestamp)
        .def("getName",&Atlas::Exchange::getName,"get unique id of the exchange");

    py::enum_<Atlas::TracerType>(m_ast, "TracerType")
        .value("NLV", Atlas::TracerType::NLV)
        .value("WEIGHTS", Atlas::TracerType::WEIGHTS)
        .value("VOLATILITY", Atlas::TracerType::VOLATILITY)
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
    py::enum_<Atlas::AST::AssetOpType>(m_ast, "AssetOpType")
        .value("ADD", Atlas::AST::AssetOpType::ADD)
        .value("SUBTRACT", Atlas::AST::AssetOpType::SUBTRACT)
        .value("MULTIPLY", Atlas::AST::AssetOpType::MULTIPLY)
        .value("DIVIDE", Atlas::AST::AssetOpType::DIVIDE)
        .export_values();
    py::enum_<Atlas::TradeLimitType>(m_ast, "TradeLimitType")
        .value("STOP_LOSS", Atlas::TradeLimitType::STOP_LOSS)
        .value("TAKE_PROFIT", Atlas::TradeLimitType::TAKE_PROFIT)
        .export_values();

    py::class_<Atlas::AST::ASTNode, std::shared_ptr<Atlas::AST::ASTNode>>(m_ast, "ASTNode");

    py::class_<Atlas::AST::StrategyBufferOpNode, Atlas::AST::ASTNode,  std::shared_ptr<Atlas::AST::StrategyBufferOpNode>>(m_ast, "StrategyBufferOpNode");

    py::class_<Atlas::AST::AssetReadNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetReadNode>>(m_ast, "AssetReadNode")
        .def_static("make", &Atlas::AST::AssetReadNode::pyMake);

    py::class_<Atlas::AST::AssetOpNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetOpNode>>(m_ast, "AssetOpNode")
        .def_static("make", &Atlas::AST::AssetOpNode::pyMake);


    py::class_<Atlas::AST::ExchangeViewFilter>(m_ast, "ExchangeViewFilter")
        .def(py::init<Atlas::AST::ExchangeViewFilterType, double, Atlas::Option<double>>(
        ));
    
    py::class_<Atlas::AST::ExchangeViewNode, Atlas::AST::StrategyBufferOpNode,  std::shared_ptr<Atlas::AST::ExchangeViewNode>>(m_ast, "ExchangeViewNode")
        .def_static("make", &Atlas::AST::ExchangeViewNode::make,
            py::arg("exchange"),
            py::arg("asset_op_node"),
            py::arg("filter") = std::nullopt);

    py::class_<Atlas::AST::EVRankNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::EVRankNode>>(m_ast, "EVRankNode")
        .def_static("make", &Atlas::AST::EVRankNode::make,
            py::arg("ev"),
            py::arg("type"),
            py::arg("count")
          );


    py::class_<Atlas::AST::TriggerNode, std::shared_ptr<Atlas::AST::TriggerNode>>(m_ast, "TriggerNode")
        .def("getMask", &Atlas::AST::TriggerNode::getMask, py::return_value_policy::reference_internal);

    py::class_<Atlas::AST::PeriodicTriggerNode, Atlas::AST::TriggerNode, std::shared_ptr<Atlas::AST::PeriodicTriggerNode>>(m_ast, "PeriodicTriggerNode")
        .def_static("make", &Atlas::AST::PeriodicTriggerNode::make,
            py::arg("exchange"),
            py::arg("frequency")
        );

    py::class_<Atlas::AST::StrategyMonthlyRunnerNode, Atlas::AST::TriggerNode, std::shared_ptr<Atlas::AST::StrategyMonthlyRunnerNode>>(m_ast, "StrategyMonthlyRunnerNode")
        .def_static("make", &Atlas::AST::StrategyMonthlyRunnerNode::make,
            py::arg("exchange"),
            py::arg("eom_trigger") = false
        );

    py::class_<Atlas::AST::TradeLimitNode, Atlas::AST::ASTNode, std::shared_ptr<Atlas::AST::TradeLimitNode>>(m_ast, "TradeLimitNode")
        .def_static("getStopLoss", &Atlas::AST::TradeLimitNode::getStopLoss)
        .def_static("setStopLoss", &Atlas::AST::TradeLimitNode::setStopLoss)
        .def_static("getTakeProfit", &Atlas::AST::TradeLimitNode::getTakeProfit)
        .def_static("setTakeProfit", &Atlas::AST::TradeLimitNode::setTakeProfit)
        .def("stopLossGetter", &Atlas::AST::TradeLimitNode::getStopLossGetter)
        .def("takeProfitGetter", &Atlas::AST::TradeLimitNode::getTakeProfitGetter)
        .def("stopLossSetter", &Atlas::AST::TradeLimitNode::getStopLossSetter)
        .def("takeProfitSetter", &Atlas::AST::TradeLimitNode::getTakeProfitSetter);

    py::class_<Atlas::AST::AllocationBaseNode, std::shared_ptr<Atlas::AST::AllocationBaseNode>>(m_ast, "AllocationBaseNode")
        .def("setTradeLimit", &Atlas::AST::AllocationBaseNode::setTradeLimit)
        .def("getTradeLimitNode", &Atlas::AST::AllocationBaseNode::getTradeLimitNode)
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
        .def(py::init<std::shared_ptr<Atlas::AST::CovarianceNodeBase>, std::optional<double>>());


    py::class_<Atlas::AST::StrategyNode, std::shared_ptr<Atlas::AST::StrategyNode>>(m_ast, "StrategyNode")
        .def("setTrigger", &Atlas::AST::StrategyNode::setTrigger)
        .def("setWarmupOverride", &Atlas::AST::StrategyNode::setWarmupOverride)
        .def_static("make", &Atlas::AST::StrategyNode::make,
            py::arg("allocation"),
            py::arg("portfolio")
        );

    py::class_<Atlas::AST::StrategyGrid, std::shared_ptr<Atlas::AST::StrategyGrid>>(m_ast, "StrategyGrid");

    py::class_<Atlas::Strategy, std::shared_ptr<Atlas::Strategy>>(m_core, "Strategy")
        .def("getNLV", &Atlas::Strategy::getNLV)
        .def("getName", &Atlas::Strategy::getName)
        .def("enableTracerHistory", &Atlas::Strategy::pyEnableTracerHistory)
        .def("setGridDimmensions", &Atlas::Strategy::pySetGridDimmensions)
        .def("setVolTracer", &Atlas::Strategy::setVolTracer)
        .def("initCommissionManager", &Atlas::Strategy::initCommissionManager)
        .def("getAllocationBuffer", &Atlas::Strategy::getAllocationBuffer, py::return_value_policy::reference_internal)
        .def("getHistory", &Atlas::Strategy::getHistory, py::return_value_policy::reference_internal)
        .def("getWeightHistory", &Atlas::Strategy::getWeightHistory, py::return_value_policy::reference_internal)
        .def(py::init<std::string, std::shared_ptr<Atlas::AST::StrategyNode>, double>());


    py::class_<Atlas::AST::GridDimension, std::shared_ptr<Atlas::AST::GridDimension>>(m_ast, "GridDimension")
        .def_static("make",
            &Atlas::AST::GridDimension::make,
            py::arg("name"),
            py::arg("dimension_values"),
            py::arg("node"),
            py::arg("getter"),
            py::arg("setter"));
        
}

int main() {
	return 0;
}
