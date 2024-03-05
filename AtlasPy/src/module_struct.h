#pragma once
#include <pybind11/pybind11.h>


namespace py = pybind11;


void wrap_order(py::module& m);
void wrap_trade(py::module& m);