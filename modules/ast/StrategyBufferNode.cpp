module;
module StrategyBufferModule;

import AllocationNodeModule;

namespace Atlas
{

namespace AST
{

Option<AllocationBaseNode*>
StrategyBufferOpNode::getAllocationNode() const noexcept {
	Option<ASTNode*> m_parent = getParent();
	while (m_parent) {
		if (m_parent.value()->getType() == NodeType::ALLOC) {
			return static_cast<AllocationBaseNode*>(*m_parent);
		}
		m_parent = m_parent.value()->getParent();
	}
	return std::nullopt;
}


}

}