module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module RankNodeModule;


import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export enum class EVRankType : uint8_t
{
	NLARGEST,	/// return the N largest
	NSMALLEST,	/// return the N smallest
	NEXTREME 	/// return the N largest and smallest
};


//============================================================================
export class EVRankNode final : public StrategyBufferOpNode
{
private:
	size_t m_N;
	EVRankType m_type;
	SharedPtr<ExchangeViewNode> m_ev;
	Vector<std::pair<size_t, double>> m_view;
	bool m_rank_in_place = false;

	void sort() noexcept;

public:
	ATLAS_API EVRankNode(
		SharedPtr<ExchangeViewNode> ev,
		EVRankType type,
		size_t count,
		bool rank_in_place = false
	) noexcept;
	ATLAS_API ~EVRankNode() noexcept;

	size_t getWarmup() const noexcept override;
	void evaluate(Eigen::VectorXd& target) noexcept override;
};

}

}