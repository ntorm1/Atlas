#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include "AtlasFeature.hpp"
#ifdef ATLAS_TORCH

import ModelBaseModule;
#include "standard/AtlasLinAlg.hpp"
#include "standard/AtlasCore.hpp"

namespace Atlas
{

namespace Model
{

class TorchModel;

//============================================================================
class TorchModelConfig
{
	friend class TorchModel;
private:
	SharedPtr<ModelConfig> m_base_config;
	String m_torch_script_file;

public:
	ATLAS_API TorchModelConfig(
		SharedPtr<ModelConfig> base_config,
		String torch_script_file
	) noexcept;
	ATLAS_API ~TorchModelConfig() noexcept = default;
};


struct TorchModelImpl;

//============================================================================
class TorchModel : public ModelBase
{
private:
	TorchModelImpl* m_impl;
	SharedPtr<const TorchModelConfig> m_torch_config;

public:
	// throws on invalid torch script file
	ATLAS_API TorchModel(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<ModelTarget> target,
		SharedPtr<const TorchModelConfig> config
	);
	ATLAS_API ~TorchModel() noexcept;

	ATLAS_API HashMap<String, Vector<float>> namedParameters() const noexcept;
	void train() noexcept override;
	void reset() noexcept override;
	void predict() noexcept override;
	[[nodiscard]] bool isSame(StrategyBufferOpNode const* other) const noexcept override;
};



}

}

#endif
