#define  _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include "module_model.h"
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

import ExchangeModule;
import LinearRegressionModule;
import ModelBaseModule;
import StrategyBufferModule;

using namespace Atlas::AST;
using namespace Atlas::Model;


//============================================================================
static void
bindModelEnum(py::module& m) {
    py::enum_<ModelTargetType>(m, "ModelTargetType")
        .value("ABSOLUTE", ModelTargetType::ABSOLUTE)
        .value("DELTA", ModelTargetType::DELTA)
        .value("PERCENTAGE_CHANGE", ModelTargetType::PERCENTAGE_CHANGE);

    py::enum_<ModelType>(m, "ModelType")
        .value("LINEAR_REGRESSION", ModelType::LINEAR_REGRESSION);
}


//============================================================================
static void
bindModelHelpers(py::module& m) {
    py::class_<ModelTarget, StrategyBufferOpNode, std::shared_ptr<ModelTarget>>(m, "ModelTarget")
        .def(py::init<std::shared_ptr<StrategyBufferOpNode>, ModelTargetType, size_t>(),
            py::arg("target"),
            py::arg("type"),
            py::arg("lookforward"));

    py::class_<ModelConfig, std::shared_ptr<ModelConfig>>(m, "ModelConfig")
        .def(py::init<size_t, size_t, ModelType, std::shared_ptr<Atlas::Exchange>>(), py::arg("training_window"), py::arg("walk_forward_window"), py::arg("model_type"), py::arg("exchange"))
        .def_readwrite("training_window", &ModelConfig::training_window)
        .def_readwrite("walk_forward_window", &ModelConfig::walk_forward_window)
        .def_readwrite("type", &ModelConfig::type);

    py::class_<ModelBase, StrategyBufferOpNode, std::shared_ptr<ModelBase>>(m, "ModelBase");
}


//============================================================================
static void bindLinearRegressionModel(py::module& m) {
    py::enum_<LinearRegressionSolver>(m, "LinearRegressionSolver")
        .value("LDLT", LinearRegressionSolver::LDLT)
        .value("ColPivHouseholderQR", LinearRegressionSolver::ColPivHouseholderQR);

    py::class_<LinearRegressionModelConfig, std::shared_ptr<LinearRegressionModelConfig>>(m, "LinearRegressionModelConfig")
        .def(py::init<std::shared_ptr<ModelConfig>, LinearRegressionSolver>(),
            py::arg("base_config"),
            py::arg("solver") = LinearRegressionSolver::LDLT)
        .def_readwrite("fit_intercept", &LinearRegressionModelConfig::m_fit_intercept)
        .def_readwrite("orthogonalize_features", &LinearRegressionModelConfig::m_orthogonalize_features);

    py::class_<LassoRegressionModelConfig, LinearRegressionModelConfig, std::shared_ptr<LassoRegressionModelConfig>>(m, "LassoRegressionModelConfig")
        .def(py::init<std::shared_ptr<ModelConfig>, double, double, size_t>(),
            py::arg("base_config"),
            py::arg("alpha") = 1.0,
            py::arg("epsilon") = 1e-4,
            py::arg("max_iter") = 1000)
        .def_readwrite("alpha", &LassoRegressionModelConfig::m_alpha)
        .def_readwrite("epsilon", &LassoRegressionModelConfig::m_epsilon)
        .def_readwrite("max_iter", &LassoRegressionModelConfig::m_max_iter);

    py::class_<LinearRegressionModel, ModelBase, std::shared_ptr<LinearRegressionModel>>(m, "LinearRegressionModel")
        .def(py::init<std::string, std::vector<std::shared_ptr<StrategyBufferOpNode>>, std::shared_ptr<ModelTarget>, std::shared_ptr<const LinearRegressionModelConfig>>(),
            py::arg("id"),
            py::arg("features"),
            py::arg("target"),
            py::arg("config"))
        .def("getX", &LinearRegressionModel::getX, py::return_value_policy::reference_internal)
        .def("getY", &LinearRegressionModel::getY, py::return_value_policy::reference_internal)
        .def("getTheta", &LinearRegressionModel::getTheta, py::return_value_policy::reference_internal);
}


void wrap_model(py::module& m_model)
{
    bindModelEnum(m_model);
    bindModelHelpers(m_model);
    bindLinearRegressionModel(m_model);
}
