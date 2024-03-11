#pragma once
#include <torch/script.h>

namespace Atlas
{

namespace Model
{

void trainTorchModule(
	torch::jit::script::Module& module,
	torch::Tensor& X_train,
	torch::Tensor& Y_train

) noexcept;


//============================================================================
std::unordered_map<std::string, std::vector<float>>
namedParamsToHashMap(
	const torch::jit::named_parameter_list& named_parameters
) noexcept;

}

}