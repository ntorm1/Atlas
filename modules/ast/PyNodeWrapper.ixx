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
	UniquePtr<T> m_node;

public:
	PyNodeWrapper(UniquePtr<T> node)
		: m_node(std::move(node))
	{
	}

	bool has_node() const
	{
		return m_node != nullptr;
	}

	UniquePtr<T> take()
	{
		auto node = std::move(m_node);
		m_node = nullptr;
		return node;
	}

};


}


}
