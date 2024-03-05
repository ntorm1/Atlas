#include "module_struct.h"
#include "AtlasStruct.hpp"



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