module;
#include <Eigen/Dense>
export module StrategyBufferModule;

import AtlasCore;
import BaseNodeModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export class StrategyBufferOpNode
	: public OpperationNode<void, Eigen::VectorXd&> {
protected:
	Exchange& m_exchange;

	StrategyBufferOpNode(
		NodeType t,
		Exchange& exchange,
		Option<ASTNode*> parent
	) :
		OpperationNode<void, Eigen::VectorXd&>(t, parent),
		m_exchange(exchange)
	{

	}

public:
	Option<AllocationBaseNode*> getAllocationNode() const noexcept;
	[[nodiscard]] Exchange& getExchange() noexcept {return m_exchange;}
};


}

}