module;
#include "AtlasFeature.hpp"

#ifdef ATLAS_TORCH

#pragma warning(push, 0)  
#include "TorchModelImpl.hpp"
#pragma warning(pop)

module TorchModule;


namespace Atlas
{

namespace Model
{

//============================================================================
TorchModelConfig::TorchModelConfig(
	SharedPtr<ModelConfig> base_config,
	String torch_script_file
) noexcept:
	m_torch_script_file(torch_script_file),
	m_base_config(base_config)
{
}


//============================================================================
struct TorchModelImpl
{
	torch::jit::script::Module torch_module;
	HashMap<String, float> coefficients;
};


//============================================================================
TorchModel::TorchModel(
	String id,
	Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
	SharedPtr<ModelTarget> target,
	SharedPtr<const TorchModelConfig> config
):
	ModelBase(id, features, target, config->m_base_config),
	m_torch_config(config)
{
	m_impl = new TorchModelImpl();

	try {
		m_impl->torch_module = torch::jit::load(config->m_torch_script_file);
	}
	catch (const c10::Error& e) {
		throw AtlasException(std::format("Error loading the torch script file: {}", e.what()));
	}
}


//============================================================================
TorchModel::~TorchModel() noexcept
{
	delete m_impl;
}


//============================================================================
HashMap<String, Vector<float>>
TorchModel::namedParameters() const noexcept
{
	return namedParamsToHashMap(m_impl->torch_module.named_parameters());
}


//============================================================================
void
TorchModel::train() noexcept
{
	int64_t look_forward = getTarget()->getLookForward();
	int64_t training_window = m_config->training_window;
	int64_t x_cols = getFeatures().size();
	
	// init torch tensors
	auto X_train = torch::zeros({ training_window - look_forward, x_cols }, torch::kF32);
	auto y_train = torch::zeros({ training_window - look_forward, 1 }, torch::kF32);
	
	// create eigen maps over tensors to copy in the training data
	float* X_train_data = X_train.data_ptr<float>();
	float* y_train_data = y_train.data_ptr<float>();
	LinAlg::EigenMap<LinAlg::EigenMatrixXf> X_train_eigen(X_train_data, training_window - look_forward, x_cols);
	LinAlg::EigenMap<LinAlg::EigenVectorXf> y_train_eigen(y_train_data, training_window - look_forward);
	copyBlocks<float, LinAlg::EigenMatrixXf, LinAlg::EigenVectorXf>(
		X_train_eigen,
		y_train_eigen
	);

	trainTorchModule(
		m_impl->torch_module,
		X_train,
		y_train
	);
}


//============================================================================
void
TorchModel::reset() noexcept
{
}


//============================================================================
void
TorchModel::predict() noexcept
{
}


//============================================================================
bool
TorchModel::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	return false;
}


}

}

#else
module TorchModule;
#endif