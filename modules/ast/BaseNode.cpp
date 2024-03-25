#include "ast/StrategyBufferNode.hpp"

namespace Atlas {

namespace AST {

//============================================================================
ASTNode::ASTNode(NodeType type,
                 Vector<SharedPtr<StrategyBufferOpNode>> parent) noexcept
    : m_type(type) {
  for (auto &p : parent) {
    m_parent.push_back(p.get());
  }
}

} // namespace AST
} // namespace Atlas