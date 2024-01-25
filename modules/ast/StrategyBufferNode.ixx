module;
export module StrategyBufferModule;

import AtlasCore;
import AtlasLinAlg;
import BaseNodeModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export class StrategyBufferOpNode
	: public OpperationNode<void, LinAlg::EigenVectorXd&> {
protected:
	Exchange& m_exchange;

	StrategyBufferOpNode(
		NodeType t,
		Exchange& exchange,
		Option<ASTNode*> parent
	) :
		OpperationNode<void, LinAlg::EigenVectorXd&>(t, parent),
		m_exchange(exchange)
	{

	}

public:
	Option<AllocationBaseNode*> getAllocationNode() const noexcept;
	[[nodiscard]] Exchange& getExchange() noexcept {return m_exchange;}
};


}

}