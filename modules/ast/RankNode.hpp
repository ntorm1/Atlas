#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif

#include "standard/AtlasCore.hpp"
#include "ast/BaseNode.hpp"
#include "ast/StrategyBufferNode.hpp"

namespace Atlas
{

namespace AST
{


//============================================================================
enum class EVRankType : uint8_t
{
	NLARGEST,	/// return the N largest
	NSMALLEST,	/// return the N smallest
	NEXTREME, 	/// return the N largest and smallest (eats signal. N Smallest set to -1, N largest to 1)
	FULL,		/// return the full rank
};


//============================================================================
class EVRankNode final : public StrategyBufferOpNode
{
private:
	size_t m_N;
	EVRankType m_type;
	SharedPtr<ExchangeViewNode> m_ev;
	Vector<std::pair<size_t, double>> m_view;
	
	void sort() noexcept;

public:
	ATLAS_API EVRankNode(
		SharedPtr<ExchangeViewNode> ev,
		EVRankType type,
		size_t count
	) noexcept;
	ATLAS_API ~EVRankNode() noexcept;


	ATLAS_API static SharedPtr<EVRankNode> make(
		SharedPtr<ExchangeViewNode> ev,
		EVRankType type,
		size_t count
	) noexcept
	{
		return std::make_shared<EVRankNode>(ev, type, count);
	}

	[[nodiscard]] size_t getN() const noexcept { return m_N; }
	[[nodiscard]] EVRankType getType() const noexcept { return m_type; }
	[[nodiscard]] SharedPtr<ExchangeViewNode> getExchangeView() const noexcept { return m_ev; }
	[[nodiscard]] size_t getWarmup() const noexcept override;
	[[nodiscard]] bool isSame(StrategyBufferOpNode const* other) const noexcept override;
	void reset() noexcept override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

}

}