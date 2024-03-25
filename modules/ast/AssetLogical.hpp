#pragma once
#define NOMINMAX
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
class AssetIfNode final
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
	
	ATLAS_API void swapRightEval(SharedPtr<StrategyBufferOpNode> right_eval) noexcept;
	ATLAS_API void swapLeftEval(SharedPtr<StrategyBufferOpNode> left_eval) noexcept;

	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] bool isSame(StrategyBufferOpNode const* other) const noexcept override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void reset() noexcept override;
};


constexpr size_t RIGHT_EVAL_IDX = 0;
constexpr size_t TRUE_EVAL_IDX = 1;
constexpr size_t FALSE_EVAL_IDX = 2;

//============================================================================
class AssetCompNode final
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
	ATLAS_API void swapTrueEval(SharedPtr<StrategyBufferOpNode> true_eval) noexcept;
	ATLAS_API void swapFalseEval(SharedPtr<StrategyBufferOpNode> false_eval) noexcept;

	[[nodiscard]] auto const& getRightEval() const noexcept { return m_right_eval; }
	[[nodiscard]] auto const& getLeftEval() const noexcept { return m_left_eval; }
	[[nodiscard]] auto const& getTrueEval() const noexcept { return m_true_eval; }
	[[nodiscard]] auto const& getFalseEval() const noexcept { return m_false_eval; }
	[[nodiscard]] LogicalType getLogicalType() const noexcept { return m_logical_type; }
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] bool isSame(StrategyBufferOpNode const* other) const noexcept override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void reset() noexcept override;
};



}

}