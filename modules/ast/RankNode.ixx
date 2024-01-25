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
	NLargest,	/// return the N largest
	NSmallest,	/// return the N smallest
};


//============================================================================
export class EVRankNode final : public StrategyBufferOpNode
{
private:
	size_t m_N;
	EVRankType m_type;
	UniquePtr<ExchangeViewNode> m_ev;
	Vector<std::pair<size_t, double>> m_view;

	void sort() noexcept;

public:
	ATLAS_API EVRankNode(
		UniquePtr<ExchangeViewNode> ev,
		EVRankType type,
		size_t count
	) noexcept;
	ATLAS_API ~EVRankNode() noexcept;

	size_t getWarmup() const noexcept override {return 0;}
	void evaluate(Eigen::VectorXd& target) noexcept override;
};

}

}