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
	SUM = 0,
	MEAN = 1,
	ATR = 2,
	MAX = 3
};


//============================================================================
export class AssetScalerNode : public StrategyBufferOpNode
{
private:
	AssetOpType m_op_type;
	double m_scale;
	SharedPtr<StrategyBufferOpNode> m_parent;

public:
	ATLAS_API AssetScalerNode(
		SharedPtr<StrategyBufferOpNode> parent,
		AssetOpType op_type,
		double scale
	) noexcept;
	ATLAS_API ~AssetScalerNode() noexcept;

	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_parent->getWarmup(); }
};


//============================================================================
export class AssetObserverNode
	: public StrategyBufferOpNode
{
	friend class Exchange;
	friend class GridDimensionObserver;
	friend class StrategyGrid;
private:
	size_t m_warmup = 0;
	size_t m_buffer_idx = 0;
	size_t m_window = 0;
	LinAlg::EigenMatrixXd m_buffer_matrix;

	SharedPtr<StrategyBufferOpNode> parent() noexcept { return m_parent; }

protected:
	void setWarmup(size_t warmup) noexcept { m_warmup = warmup; }
	[[nodiscard]] size_t getBufferIdx() const noexcept { return m_buffer_idx; }
	[[nodiscard]] auto const& getBufferMatrix() const noexcept { return m_buffer_matrix; }
	[[nodiscard]] size_t getWindow() const noexcept { return m_window; }

public:
	SharedPtr<StrategyBufferOpNode> m_parent;
	AssetObserverType m_observer_type;

	void resetBase() noexcept;
	void cacheBase() noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	[[nodiscard]] AssetObserverType observerType() const noexcept { return m_observer_type; }
	[[nodiscard]] size_t window() const noexcept { return m_window; }
	
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer() noexcept;
	virtual void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept = 0;
	virtual void cacheObserver() noexcept = 0;
	virtual size_t hash() const noexcept = 0;
	virtual void reset() noexcept = 0;

	AssetObserverNode(
		SharedPtr<StrategyBufferOpNode> parent,
		AssetObserverType observer_type,
		size_t window
	) noexcept;
	ATLAS_API virtual ~AssetObserverNode() noexcept;
};


//============================================================================
export class SumObserverNode final
	: public AssetObserverNode
{
private:
	LinAlg::EigenVectorXd m_sum;

public:
	[[nodiscard]] size_t hash() const noexcept override;
	[[nodiscard]] size_t refreshWarmup() noexcept override { return getWarmup(); }

	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;

	ATLAS_API SumObserverNode(
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~SumObserverNode() noexcept;
};


//============================================================================
export class MeanObserverNode final
	: public AssetObserverNode
{
private:
	SharedPtr<SumObserverNode> m_sum_observer;
	SharedPtr<AssetScalerNode> m_scaler;

public:
	ATLAS_API MeanObserverNode(
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~MeanObserverNode() noexcept;

	[[nodiscard]] size_t hash() const noexcept override;
	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
	[[nodiscard]] size_t refreshWarmup() noexcept override;
};


//============================================================================
export class MaxObserverNode final
	: public AssetObserverNode
{
private:
	LinAlg::EigenVectorXd m_max;

public:
	ATLAS_API MaxObserverNode(
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~MaxObserverNode() noexcept;

	[[nodiscard]] size_t hash() const noexcept override;
	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void cacheObserver() noexcept override;
};


}

}