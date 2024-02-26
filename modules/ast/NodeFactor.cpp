module;
module NodeFactoryModule;


namespace Atlas
{

namespace AST
{

//============================================================================
NodeFactory::NodeFactory(
	String strategy_id,
	SharedPtr<Exchange> exchange
) noexcept:
	m_strategy_id(strategy_id),
	m_exchange(exchange)
{
}


//============================================================================
NodeFactory::~NodeFactory() noexcept
{
}

}

}