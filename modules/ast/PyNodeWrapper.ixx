module;
#pragma once
#include <cassert>
export module PyNodeWrapperModule;


import AtlasCore;

namespace Atlas
{

namespace AST
{


export template <typename T>
class PyNodeWrapper
{
private:
	mutable UniquePtr<T> m_node;

public:
	PyNodeWrapper(UniquePtr<T> node)
		: m_node(std::move(node))
	{
	}

	// pybind11 needs a default constructor to be able to create a py::object
	PyNodeWrapper() = default;
	
	// copy constructor
	PyNodeWrapper(const PyNodeWrapper& other)
		: m_node(std::move(other.m_node))
	{
	}

	// Move Constructor
	PyNodeWrapper(PyNodeWrapper&& other) noexcept
		: m_node(std::move(other.m_node))
	{
	}

	// Copy Assignment Operator
	PyNodeWrapper& operator=(const PyNodeWrapper& other)
	{
		if (this != &other)
		{
			m_node = other.m_node ? std::move(other.m_node) : nullptr;
		}
		return *this;
	}

	// Move Assignment Operator
	PyNodeWrapper& operator=(PyNodeWrapper&& other) noexcept
	{
		if (this != &other)
		{
			m_node = std::move(other.m_node);
		}
		return *this;
	}

	bool has_node() const
	{
		return m_node != nullptr;
	}

	UniquePtr<T> take()
	{
		assert(has_node());
		auto node = std::move(m_node);
		m_node = nullptr;
		return node;
	}

};


}


}
