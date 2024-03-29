#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include "standard/AtlasCore.hpp"
#include "standard/AtlasEnums.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "ast/BaseNode.hpp"
#include "ast/StrategyBufferNode.hpp"

namespace Atlas
{

namespace AST
{


//============================================================================
enum class AllocationType
{
	UNIFORM = 0,
	CONDITIONAL_SPLIT = 1,
	FIXED = 2,
	NLARGEST = 3,
	NSMALLEST = 4,
	NEXTREME = 5,
};


struct AllocationBaseNodeImpl;

//============================================================================
class AllocationBaseNode : public StrategyBufferOpNode
{
	friend class StrategyNode;
protected:
	UniquePtr<AllocationBaseNodeImpl> m_impl;
	size_t m_warmup = 0;
	bool copy_weights_buffer = false;
	void reset() noexcept override;
	void setWarmup(size_t warmup) noexcept { m_warmup = warmup; }
	void setTracer(SharedPtr<Tracer> tracer) noexcept;
	void setCopyWeightsBuffer(bool copy) noexcept { copy_weights_buffer = copy; }
	[[nodiscard]] LinAlg::EigenRef<LinAlg::EigenVectorXd> getPnL() noexcept;

public:
	virtual ~AllocationBaseNode() noexcept;
	AllocationBaseNode(
		AllocationType m_type,
		Exchange& exchange,
		double epsilon,
		Option<double> alloc_param
	) noexcept;

	[[nodiscard]] AllocationType getType() const noexcept;
	[[nodiscard]] double getAllocEpsilon() const noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept { return m_warmup; }
	[[nodiscard]] size_t getAssetCount() const noexcept;
	[[nodiscard]] bool isSame(StrategyBufferOpNode const* other) const noexcept final override { return false; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	virtual void evaluateChild(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept = 0;

	size_t internalRefCount() const noexcept;
	ATLAS_API Option<SharedPtr<TradeLimitNode>> getTradeLimitNode() const noexcept;
	ATLAS_API void setWeightScale(SharedPtr<AllocationWeightNode> scale) noexcept;
	ATLAS_API void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept;
	ATLAS_API void setTradeLimit(TradeLimitType t, double limit) noexcept;
};


//============================================================================
class FixedAllocationNode final
	: public AllocationBaseNode
{
private:
	Vector<std::pair<size_t, double>> m_allocations;

public:
	//============================================================================
	ATLAS_API ~FixedAllocationNode() noexcept;

	//============================================================================
	ATLAS_API FixedAllocationNode(
		Vector<std::pair<size_t, double>> m_allocations,
		Exchange* exchange,
		double epsilon = 0.000f
	) noexcept;


	ATLAS_API [[nodiscard]] static Result<SharedPtr<FixedAllocationNode>, AtlasException>
	make(
			Vector<std::pair<String, double>> m_allocations,
			Exchange* exchange,
			double epsilon = 0.000f
	) noexcept;

	ATLAS_API [[nodiscard]] static SharedPtr<FixedAllocationNode>
	pyMake(
		Vector<std::pair<String, double>> m_allocations,
		Exchange* exchange,
		double epsilon = 0.000f
	);

	[[nodiscard]] size_t refreshWarmup() noexcept override {return 0;}
	void evaluateChild(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};


//============================================================================
class AllocationNode
final
	: public AllocationBaseNode
{
private:
	Option<size_t> n_alloc_param = std::nullopt;
	SharedPtr<StrategyBufferOpNode> m_exchange_view;
public:
	ATLAS_API ~AllocationNode() noexcept;

	void setNAllocParam(size_t n) noexcept { n_alloc_param = n; }
	[[nodiscard]] size_t getNAllocParam() const noexcept { return n_alloc_param.value(); }

	ATLAS_API AllocationNode(
		SharedPtr<StrategyBufferOpNode> exchange_view,
		AllocationType type = AllocationType::UNIFORM,
		Option<double> alloc_param = std::nullopt,
		double epsilon = 0.000f
	) noexcept;

	ATLAS_API [[nodiscard]] static Result<SharedPtr<AllocationNode>, AtlasException>
	make(
			SharedPtr<StrategyBufferOpNode> exchange_view,
			AllocationType type = AllocationType::UNIFORM,
			Option<double> alloc_param = std::nullopt,
			double epsilon = 0.000f
	) noexcept;

	ATLAS_API [[nodiscard]] static SharedPtr<AllocationNode>
	pyMake(
			SharedPtr<StrategyBufferOpNode> exchange_view,
			AllocationType type = AllocationType::UNIFORM,
			Option<double> alloc_param = std::nullopt,
			double epsilon = 0.000f
	);

	[[nodiscard]] size_t refreshWarmup() noexcept override;
	void evaluateChild(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};


}

}