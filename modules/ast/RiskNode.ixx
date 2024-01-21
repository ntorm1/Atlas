module;

export module RiskNodeModule;


import AtlasLinAlg;
import AtlasCore;
import BaseNodeModule;
import AtlasTimeModule;

namespace Atlas
{

namespace AST
{



//============================================================================
struct RiskLookbackDef
{
	size_t trigger_call_idx;
	size_t returns_start_idx;
	size_t returns_end_idx;

	RiskLookbackDef(
		size_t trigger_call_idx,
		size_t returns_start_idx,
		size_t returns_end_idx
	) noexcept :
		trigger_call_idx(trigger_call_idx),
		returns_start_idx(returns_start_idx),
		returns_end_idx(returns_end_idx)
	{}
};


//============================================================================
export class AllocationWeightNode : public OpperationNode<void, LinAlg::EigenVectorXd&>
{
protected:
	Time::TimeOffset m_lookback;
	SharedPtr<TriggerNode> m_trigger;
	Vector<RiskLookbackDef> m_lookback_defs;
	Exchange const& m_exchange;
	void buildLookbackDefs() noexcept;

public:
	virtual ~AllocationWeightNode() noexcept;
	AllocationWeightNode(
		Time::TimeOffset l,
		SharedPtr<TriggerNode> trigger
	) noexcept;

	virtual void cache() noexcept = 0;
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;

};


//============================================================================
export class InvVolWeight final : public AllocationWeightNode
{
private:
	LinAlg::EigenVectorXd m_vol;
	size_t m_lookback_idx = 0;
public:
	~InvVolWeight() noexcept;
	InvVolWeight(
		Time::TimeOffset l,
		SharedPtr<TriggerNode> trigger
	) noexcept;

	void cache() noexcept override;
	void evaluate(LinAlg::EigenVectorXd& target) noexcept override = 0;

};


}

}
