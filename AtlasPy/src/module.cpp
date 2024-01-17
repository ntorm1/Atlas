#include <expected>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

import HydraModule;
import ExchangeModule;
import AtlasException;
import AtlasCore;

namespace py = pybind11;

using namespace Atlas;


template <class T> class ptr_wrapper
{
public:
    ptr_wrapper() : ptr(nullptr) {}
    ptr_wrapper(T* ptr) : ptr(ptr) {}
    ptr_wrapper(const ptr_wrapper& other) : ptr(other.ptr) {}
    T& operator* () const { return *ptr; }
    T* operator->() const { return  ptr; }
    T* get() const { return ptr; }
    void destroy() { delete ptr; }
    T& operator[](std::size_t idx) const { return ptr[idx]; }
private:
    T* ptr;
};


//============================================================================
class PyExchange {
private:
    Exchange* exchange = nullptr;

public:
    PyExchange(Exchange* exchange) noexcept : exchange(exchange) {}

    String getName() {
        return exchange->getName();
    }

};


//============================================================================
class PyHydra {
private:
    SharedPtr<Hydra> hydra = nullptr;
public:

    PyHydra() noexcept {
	    hydra = std::make_shared<Hydra>();
    }

    void dummy() {}

    PyExchange addExchange(
        String name,
        String source
    )
    {
        auto res = hydra->addExchange(name, source);
        if(!res.has_value()) 
        {
			throw std::runtime_error(res.error().what());
		}
		return PyExchange(res.value());
    }
};





PYBIND11_MODULE(AtlasPy, m) {
    auto m_core = m.def_submodule("core");

    
    py::class_<PyHydra>(m_core, "Hydra")
        .def("addExchange", &PyHydra::addExchange)
        .def("dummy", &PyHydra::dummy)
        .def(py::init<>());

    py::class_<PyExchange>(m_core, "Exchange")
		.def("getName", &PyExchange::getName)
		.def(py::init<Exchange*>());
}

int main() {
	return 0;
}
