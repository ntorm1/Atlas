module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module AssetLogicalModule;

import AtlasCore;
import AtlasEnumsModule;
import AtlasLinAlg;
import BaseNodeModule;
import StrategyBufferModule;

namespace Atlas
{

namespace AST
{

//============================================================================
export class AssetIfNode final
	: public StrategyBufferOpNode
{
private:
	LinAlg::EigenVectorXd m_buffer;
	size_t m_warmup = 0;
	AssetCompType m_comp_type;
	SharedPtr<StrategyBufferOpNode> m_right_eval;
	SharedPtr<StrategyBufferOpNode> m_left_eval;

public:
	ATLAS_API AssetIfNode(
		SharedPtr<StrategyBufferOpNode> left_eval,
		AssetCompType comp_type,
		SharedPtr<StrategyBufferOpNode> right_eval
	) noexcept;
	ATLAS_API ~AssetIfNode() noexcept;
	ATLAS_API void swapRightEval(SharedPtr<StrategyBufferOpNode> right_eval) noexcept { m_right_eval = right_eval; }
	ATLAS_API void swapLeftEval(SharedPtr<StrategyBufferOpNode> left_eval) noexcept { m_left_eval = left_eval; }

	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;

};

constexpr size_t RIGHT_EVAL_IDX = 0;
constexpr size_t TRUE_EVAL_IDX = 1;
constexpr size_t FALSE_EVAL_IDX = 2;

//============================================================================
export class AssetCompNode final
	: public StrategyBufferOpNode
{
private:
	LinAlg::EigenMatrixXd m_buffer;
	LogicalType m_logical_type;
	size_t m_warmup = 0;
	SharedPtr<StrategyBufferOpNode> m_right_eval;
	SharedPtr<StrategyBufferOpNode> m_left_eval;
	SharedPtr<StrategyBufferOpNode> m_true_eval;
	SharedPtr<StrategyBufferOpNode> m_false_eval;

public:
	ATLAS_API AssetCompNode(
		SharedPtr<StrategyBufferOpNode> left_eval,
		LogicalType logicial_type,
		SharedPtr<StrategyBufferOpNode> right_eval,
		SharedPtr<StrategyBufferOpNode> true_eval,
		SharedPtr<StrategyBufferOpNode> false_eval
	) noexcept;
	ATLAS_API ~AssetCompNode() noexcept;
	ATLAS_API void swapTrueEval(SharedPtr<StrategyBufferOpNode> true_eval) noexcept { m_true_eval = true_eval; }
	ATLAS_API void swapFalseEval(SharedPtr<StrategyBufferOpNode> false_eval) noexcept { m_false_eval = false_eval; }

	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};



}

}