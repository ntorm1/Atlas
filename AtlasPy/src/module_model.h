#pragma once
#include <pybind11/pybind11.h>


namespace py = pybind11;


void wrap_model(py::module& m_model);