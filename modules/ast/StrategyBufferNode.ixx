module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
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
	: public OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>> {
	friend class Exchange;

private:

	void setTakeFromCache(bool v) noexcept;

protected:
	Exchange& m_exchange;
	LinAlg::EigenMatrixXd m_cache;
	bool m_take_from_cache = false;

	StrategyBufferOpNode(
		NodeType t,
		Exchange& exchange,
		Option<ASTNode*> parent
	) :
		OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>>(t, parent),
		m_exchange(exchange)
	{

	}

	void enableCache(bool v = true) noexcept;
	bool hasCache() const noexcept { return m_cache.cols() > 1; }
	LinAlg::EigenRef<LinAlg::EigenVectorXd> cacheColumn() noexcept;

public:
	virtual ~StrategyBufferOpNode() = default;
	virtual bool isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept { return false; }
	virtual void reset() noexcept {}
	virtual size_t refreshWarmup() noexcept {return 0;}
	Option<AllocationBaseNode*> getAllocationNode() const noexcept;
	[[nodiscard]] Exchange& getExchange() noexcept {return m_exchange;}
	ATLAS_API auto const& cache() noexcept {return m_cache;}
};


}

}