module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ObserverNodeModule;


import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;
import ObserverNodeBaseModule;
import AtlasLinAlg;

namespace Atlas
{

namespace AST
{


//============================================================================
export class SumObserverNode final
	: public AssetObserverNode
{
public:
	ATLAS_API SumObserverNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~SumObserverNode() noexcept;


	[[nodiscard]] size_t refreshWarmup() noexcept override { return getWarmup(); }
	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
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
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~MeanObserverNode() noexcept;

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
public:
	ATLAS_API MaxObserverNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~MaxObserverNode() noexcept;
	
	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
};


//============================================================================
export class TsArgMaxObserverNode final
	: public AssetObserverNode
{
private:
	SharedPtr<MaxObserverNode> m_max_observer;
public:
	ATLAS_API TsArgMaxObserverNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~TsArgMaxObserverNode() noexcept;

	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
};


//============================================================================
export class VarianceObserverNode final
	: public AssetObserverNode
{
private:
	SharedPtr<MeanObserverNode> m_mean_observer;
	SharedPtr<SumObserverNode> m_sum_squared_observer;
public:
	ATLAS_API VarianceObserverNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~VarianceObserverNode() noexcept;

	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
};


//============================================================================
export class CovarianceObserverNode final
	: public AssetObserverNode
{
private:
	SharedPtr<StrategyBufferOpNode> m_right_parent;
	SharedPtr<SumObserverNode> m_left_sum_observer;
	SharedPtr<SumObserverNode> m_right_sum_observer;
	SharedPtr<SumObserverNode> m_cross_sum_observer;
public:
	ATLAS_API CovarianceObserverNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> left_parent,
		SharedPtr<StrategyBufferOpNode> right_parent,
		size_t window
	) noexcept;
	ATLAS_API ~CovarianceObserverNode() noexcept;

	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
};


//============================================================================
export class CorrelationObserverNode final
	: public AssetObserverNode
{
private:
	SharedPtr<StrategyBufferOpNode> m_right_parent;
	SharedPtr<CovarianceObserverNode> m_cov_observer;
	SharedPtr<VarianceObserverNode> m_left_var_observer;
	SharedPtr<VarianceObserverNode> m_right_var_observer;

public:
	ATLAS_API CorrelationObserverNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> left_parent,
		SharedPtr<StrategyBufferOpNode> right_parent,
		size_t window
	) noexcept;
	ATLAS_API ~CorrelationObserverNode() noexcept;

	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
};


//============================================================================
export class LinearDecayNode final
	: public AssetObserverNode
{
private:
	bool is_first_step = true;
	double m_alpha;
	LinAlg::EigenVectorXd m_decay_buffer;
public:
	ATLAS_API LinearDecayNode(
		Option<String> id,
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~LinearDecayNode() noexcept;

	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
	void cacheObserver() noexcept override;
	void reset() noexcept override;
};


}

}