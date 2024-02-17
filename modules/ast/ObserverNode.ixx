module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module ObserverNodeModule;


import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;
import AtlasLinAlg;

namespace Atlas
{

namespace AST
{


class AssetObserverImpl;


//============================================================================
export enum class AssetObserverType : Uint8
{
	SUM = 0
};


//============================================================================
export class AssetScalerNode : public StrategyBufferOpNode
{
private:
	AssetOpType m_op_type;
	double m_scale;
	SharedPtr<StrategyBufferOpNode> m_parent;

public:
	AssetScalerNode(
		SharedPtr<StrategyBufferOpNode> parent,
		AssetOpType op_type,
		double scale
	) noexcept;
	~AssetScalerNode() noexcept;

	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_parent->getWarmup(); }
};


//============================================================================
export class AssetObserverNode
	: public StrategyBufferOpNode
{
private:
	AssetObserverType m_observer_type;
	size_t m_buffer_idx = 0;
	size_t m_window = 0;
	LinAlg::EigenMatrixXd m_buffer_matrix;
protected:
	SharedPtr<StrategyBufferOpNode> m_parent;

	void cacheBase() noexcept;
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer() noexcept;
	virtual void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept = 0;
	virtual void cache() noexcept = 0;

	AssetObserverNode(
		Exchange& exchange,
		SharedPtr<StrategyBufferOpNode> parent,
		AssetObserverType observer_type,
		size_t window
	) noexcept;
	~AssetObserverNode() noexcept;
};


//============================================================================
export class SumObserverNode final
	: public AssetObserverNode
{
private:
	size_t m_window;
	LinAlg::EigenVectorXd m_sum;

public:
	SumObserverNode(
		SharedPtr<Exchange> exchange,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	~SumObserverNode() noexcept;

	[[nodiscard]] size_t getWarmup() const noexcept override { return m_window; }
	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void cache() noexcept;
};

}

}