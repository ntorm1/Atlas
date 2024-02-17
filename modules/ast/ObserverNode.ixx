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
private:
	size_t m_buffer_idx = 0;
	size_t m_window = 0;
	LinAlg::EigenMatrixXd m_buffer_matrix;
protected:
	AssetObserverType m_observer_type;
	SharedPtr<StrategyBufferOpNode> m_parent;

	void resetBase() noexcept;
	void cacheBase() noexcept;
	
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer() noexcept;
	virtual void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept = 0;
	virtual void cache() noexcept = 0;
	virtual size_t hash() const noexcept = 0;
	virtual void reset() noexcept = 0;

public:
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
	size_t m_window;
	LinAlg::EigenVectorXd m_sum;

protected:
	[[nodiscard]] size_t hash() const noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_window; }
	void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void cache() noexcept;
	void reset() noexcept;

public:

	ATLAS_API SumObserverNode(
		SharedPtr<StrategyBufferOpNode> parent,
		size_t window
	) noexcept;
	ATLAS_API ~SumObserverNode() noexcept;


};

}

}