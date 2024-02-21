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
	LinAlg::EigenRef<LinAlg::EigenVectorXd> cacheColumn(Option<size_t> col = std::nullopt) noexcept;

public:
	virtual ~StrategyBufferOpNode() = default;
	virtual bool isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept { return false; }
	virtual void reset() noexcept {}
	virtual size_t refreshWarmup() noexcept {return 0;}
	[[nodiscard]] size_t getAssetCount() const noexcept;
	[[nodiscard]] Option<AllocationBaseNode*> getAllocationNode() const noexcept;
	[[nodiscard]] Exchange& getExchange() noexcept {return m_exchange;}

	ATLAS_API SharedPtr<StrategyBufferOpNode> lag(size_t lag) noexcept;
	ATLAS_API Option<Vector<double>> getAssetCacheSlice(size_t asset_index) const noexcept;
	ATLAS_API auto const& cache() noexcept {return m_cache;}
};


//============================================================================
export class DummyNode : public StrategyBufferOpNode {
public:
	ATLAS_API DummyNode(SharedPtr<Exchange> exchange) noexcept :
		StrategyBufferOpNode(NodeType::NOP, *exchange, std::nullopt)
	{}
	ATLAS_API ~DummyNode() noexcept {}

	[[nodiscard]] size_t getWarmup() const noexcept override { return 0; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override {}
};


//============================================================================
export class LagNode : public StrategyBufferOpNode {
private:
	size_t m_lag;
	size_t m_lag_cache_idx = 0;
	StrategyBufferOpNode* m_parent;
public:
	LagNode(
		StrategyBufferOpNode* parent,
		size_t lag
	) noexcept;
	ATLAS_API ~LagNode() noexcept {}

	ATLAS_API [[nodiscard]] size_t getWarmup() const noexcept override;
	ATLAS_API void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

}

}