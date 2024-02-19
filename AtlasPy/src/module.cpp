#define  _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
#include <expected>
#include <optional>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include "AtlasStruct.hpp"

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
import ObserverNodeModule;
import RankNodeModule;
import StrategyModule;
import StrategyNodeModule;
import StrategyBufferModule;
import RiskNodeModule;
import TradeNodeModule;
import TracerModule;


namespace py = pybind11;

// Wrapper for the Order struct
void wrap_order(py::module& m) {
    py::class_<Atlas::Order>(m, "Order")
        .def(py::init<size_t, size_t, long long, double, double>())
        .def_readonly("asset_id", &Atlas::Order::asset_id)
        .def_readonly("strategy_id", &Atlas::Order::strategy_id)
        .def_readonly("fill_time", &Atlas::Order::fill_time)
        .def_readonly("quantity", &Atlas::Order::quantity)
        .def_readonly("fill_price", &Atlas::Order::fill_price)
        .def("to_dict", 
            [](const Atlas::Order& order) {
                py::dict d;
                d["asset_id"] = order.asset_id;
                d["strategy_id"] = order.strategy_id;
                d["fill_time"] = order.fill_time;
                d["quantity"] = order.quantity;
                d["fill_price"] = order.fill_price;
                return d;
            });
}

// Wrapper for the Trade struct
void wrap_trade(py::module& m) {
    py::class_<Atlas::Trade>(m, "Trade")
        .def(py::init<size_t, size_t, long long, long long, double, double, double>())
        .def_readonly("asset_id", &Atlas::Trade::asset_id)
        .def_readonly("strategy_id", &Atlas::Trade::strategy_id)
        .def_readonly("open_time", &Atlas::Trade::open_time)
        .def_readonly("close_time", &Atlas::Trade::close_time)
        .def_readonly("quantity", &Atlas::Trade::quantity)
        .def_readonly("open_price", &Atlas::Trade::open_price)
        .def_readonly("close_price", &Atlas::Trade::close_price)
        .def("to_dict",
			[](const Atlas::Trade& trade) {
				py::dict d;
				d["asset_id"] = trade.asset_id;
				d["strategy_id"] = trade.strategy_id;
				d["open_time"] = trade.open_time;
				d["close_time"] = trade.close_time;
				d["quantity"] = trade.quantity;
				d["open_price"] = trade.open_price;
				d["close_price"] = trade.close_price;
				return d;
			});
}


PYBIND11_MODULE(AtlasPy, m) {
    auto m_core = m.def_submodule("core");
    auto m_ast = m.def_submodule("ast");

    wrap_order(m_core);
    wrap_trade(m_core);

    // ======= PYTHON API ======= //
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

    py::class_<Atlas::AST::ASTNode, std::shared_ptr<Atlas::AST::ASTNode>>(m_ast, "ASTNode");
    
    py::class_<Atlas::AST::StrategyBufferOpNode, Atlas::AST::ASTNode, std::shared_ptr<Atlas::AST::StrategyBufferOpNode>>(m_ast, "StrategyBufferOpNode")
        .def("enableCache", &Atlas::AST::StrategyBufferOpNode::enableCache,
            py::arg("v") = true)
        .def("cache", &Atlas::AST::StrategyBufferOpNode::cache, py::return_value_policy::reference_internal);
    
    py::class_<Atlas::AST::AssetObserverNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetObserverNode>>(m_ast, "AssetObserverNode");


    py::class_<Atlas::Exchange, std::shared_ptr<Atlas::Exchange>>(m_core, "Exchange")
        .def("registerObserver", &Atlas::Exchange::registerObserver)
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
        .value("ORDERS_EAGER", Atlas::TracerType::ORDERS_EAGER)
        .export_values();

    // ======= AST API ======= //
    py::enum_<Atlas::AST::ExchangeViewFilterType>(m_ast, "ExchangeViewFilterType")
        .value("GREATER_THAN", Atlas::AST::ExchangeViewFilterType::GREATER_THAN)
        .value("LESS_THAN", Atlas::AST::ExchangeViewFilterType::LESS_THAN)
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
    py::enum_<Atlas::GridType>(m_ast, "GridType")
        .value("UPPER_TRIANGULAR", Atlas::GridType::UPPER_TRIANGULAR)
        .value("LOWER_TRIANGULAR", Atlas::GridType::LOWER_TRIANGULAR)
        .export_values();


    py::class_<Atlas::AST::AssetReadNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetReadNode>>(m_ast, "AssetReadNode")
        .def_static("make", &Atlas::AST::AssetReadNode::pyMake);

    py::class_<Atlas::AST::AssetMedianNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetMedianNode>>(m_ast, "AssetMedianNode")
        .def_static("make", &Atlas::AST::AssetMedianNode::pyMake);

    py::class_<Atlas::AST::ATRNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::ATRNode>>(m_ast, "ATRNode")
        .def("getATR", &Atlas::AST::ATRNode::getATR, py::return_value_policy::reference_internal)
        .def_static("make", &Atlas::AST::ATRNode::pyMake);

    py::class_<Atlas::AST::AssetOpNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetOpNode>>(m_ast, "AssetOpNode")
        .def("getSwapLeft", &Atlas::AST::AssetOpNode::getSwapLeft)
        .def("getSwapRight", &Atlas::AST::AssetOpNode::getSwapRight)
        .def_static("make", &Atlas::AST::AssetOpNode::pyMake);


    py::class_<Atlas::AST::ExchangeViewFilter, std::shared_ptr<Atlas::AST::ExchangeViewFilter>>(m_ast, "ExchangeViewFilter")
        .def(py::init<Atlas::AST::ExchangeViewFilterType, double, Atlas::Option<double>>(
        ));
    
    py::class_<Atlas::AST::ExchangeViewNode, Atlas::AST::StrategyBufferOpNode,  std::shared_ptr<Atlas::AST::ExchangeViewNode>>(m_ast, "ExchangeViewNode")
        .def_static("make", &Atlas::AST::ExchangeViewNode::make,
            py::arg("exchange"),
            py::arg("asset_op_node"),
            py::arg("filter") = std::nullopt,
            py::arg("left_view") = std::nullopt);

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

    py::class_<Atlas::Tracer, std::shared_ptr<Atlas::Tracer>>(m_ast, "Tracer")
        .def("getOrders", &Atlas::Tracer::getOrders, py::return_value_policy::reference_internal)
        .def("getHistory", &Atlas::Tracer::getHistory, py::return_value_policy::reference_internal);

    py::class_<Atlas::AST::AssetScalerNode, Atlas::AST::StrategyBufferOpNode, std::shared_ptr<Atlas::AST::AssetScalerNode>>(m_ast, "AssetScalerNode")
       .def(py::init<std::shared_ptr<Atlas::AST::StrategyBufferOpNode>, Atlas::AST::AssetOpType, double>());

    py::class_<Atlas::AST::SumObserverNode, Atlas::AST::AssetObserverNode, std::shared_ptr<Atlas::AST::SumObserverNode>>(m_ast, "SumObserverNode")
        .def(py::init<std::shared_ptr<Atlas::AST::StrategyBufferOpNode>, size_t>());

    py::class_<Atlas::AST::MeanObserverNode, Atlas::AST::AssetObserverNode, std::shared_ptr<Atlas::AST::MeanObserverNode>>(m_ast, "MeanObserverNode")
        .def(py::init<std::shared_ptr<Atlas::AST::StrategyBufferOpNode>, size_t>());

    py::class_<Atlas::AST::StrategyGrid, std::shared_ptr<Atlas::AST::StrategyGrid>>(m_ast, "StrategyGrid")
        .def("enableTracerHistory", &Atlas::AST::StrategyGrid::enableTracerHistory)
        .def("getTracer", &Atlas::AST::StrategyGrid::getTracer)
        .def("rows", &Atlas::AST::StrategyGrid::rows)
        .def("cols", &Atlas::AST::StrategyGrid::cols)
        .def("meanReturn", &Atlas::AST::StrategyGrid::meanReturn);

    py::class_<Atlas::Strategy, std::shared_ptr<Atlas::Strategy>>(m_core, "Strategy")
        .def("getNLV", &Atlas::Strategy::getNLV)
        .def("getName", &Atlas::Strategy::getName)
        .def("enableTracerHistory", &Atlas::Strategy::pyEnableTracerHistory)
        .def("setGridDimmensions", &Atlas::Strategy::pySetGridDimmensions,
            py::arg("dimensions"),
            py::arg("grid_type") = std::nullopt
            )
        .def("getTracer", &Atlas::Strategy::getTracer, py::return_value_policy::reference_internal)
        .def("setVolTracer", &Atlas::Strategy::setVolTracer)
        .def("initCommissionManager", &Atlas::Strategy::initCommissionManager)
        .def("getAllocationBuffer", &Atlas::Strategy::getAllocationBuffer, py::return_value_policy::reference_internal)
        .def("getHistory", &Atlas::Strategy::getHistory, py::return_value_policy::reference_internal)
        .def("getWeightHistory", &Atlas::Strategy::getWeightHistory, py::return_value_policy::reference_internal)
        .def(py::init<std::string, std::shared_ptr<Atlas::AST::StrategyNode>, double>());

    py::class_<Atlas::AST::GridDimension, std::shared_ptr<Atlas::AST::GridDimension>>(m_ast, "GridDimension");

    py::class_<Atlas::AST::GridDimensionObserver, Atlas::AST::GridDimension, std::shared_ptr<Atlas::AST::GridDimensionObserver>>(m_ast, "GridDimensionObserver")
        .def_static("make",
            &Atlas::AST::GridDimensionObserver::make,
            py::arg("name"),
            py::arg("dimension_values"),
            py::arg("observer_base"),
            py::arg("observer_child"),
            py::arg("swap_addr")
        );

    py::class_<Atlas::AST::GridDimensionLimit, Atlas::AST::GridDimension, std::shared_ptr<Atlas::AST::GridDimensionLimit>>(m_ast, "GridDimensionLimit")
        .def_static("make",
            &Atlas::AST::GridDimensionLimit::make,
            py::arg("name"),
            py::arg("dimension_values"),
            py::arg("node"),
            py::arg("getter"),
            py::arg("setter"));
        
}

int main() {
	return 0;
}
