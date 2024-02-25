module;
module ModelNodeModule;

namespace Atlas
{

namespace AST
{

//============================================================================
struct ModelNodeImpl
{

};


//============================================================================
ModelNode::ModelNode(
	Exchange& exchange
) noexcept :
	StrategyBufferOpNode(NodeType::MODEL, exchange, std::nullopt)
{
	m_impl = new ModelNodeImpl();
}


//============================================================================
ModelNode::~ModelNode() noexcept
{
	delete m_impl;
}

}


}