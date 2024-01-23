module;
#include <Eigen/Dense>
export module StrategyBufferModule;

import AtlasCore;
import BaseNodeModule;

namespace Atlas
{

namespace AST
{


//============================================================================
export class StrategyBufferOpNode
	: public OpperationNode<void, Eigen::VectorXd&> {
protected:
	Exchange& m_exchange;

	StrategyBufferOpNode(
		NodeType t,
		Exchange& exchange,
		Option<ASTNode*> parent
	) :
		OpperationNode<void, Eigen::VectorXd&>(t, parent),
		m_exchange(exchange)
	{

	}

public:
	Option<AllocationBaseNode*> getAllocationNode() const noexcept;
	[[nodiscard]] Exchange& getExchange() noexcept {return m_exchange;}
};


export enum class AllocationType
{
	UNIFORM = 0,
	CONDITIONAL_SPLIT = 1,
	FIXED = 2
};


//============================================================================
export class AllocationBaseNode : public StrategyBufferOpNode
{
protected:
	AllocationType m_type;
	double m_epsilon;
	Eigen::VectorXd m_weights_buffer;
	Option<double> m_alloc_param = std::nullopt;
	Option<SharedPtr<CommisionManager>> m_commision_manager = std::nullopt;
public:
	virtual ~AllocationBaseNode() noexcept = default;
	AllocationBaseNode(
		AllocationType m_type,
		Exchange& exchange,
		double epsilon,
		Option<double> alloc_param
	) noexcept;

	[[nodiscard]] AllocationType getType() const noexcept { return m_type; }
	[[nodiscard]] double getAllocEpsilon() const noexcept { return m_epsilon; }
	[[nodiscard]] virtual size_t getWarmup() const noexcept = 0;
	void setCommissionManager(SharedPtr<CommisionManager> manager) noexcept { m_commision_manager = manager; }
	void evaluate(Eigen::VectorXd& target) noexcept override;
	virtual void evaluateChild(Eigen::VectorXd& target) noexcept = 0;
};


}

}