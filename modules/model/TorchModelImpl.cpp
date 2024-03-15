#include "AtlasFeature.hpp"

#ifdef ATLAS_TORCH

#pragma warning(push, 0)
#include "TorchModelImpl.hpp"
#include <torch/torch.h>
#pragma warning(pop)

namespace Atlas {

namespace Model {

//============================================================================
void trainTorchModule(torch::jit::script::Module &torch_module,
                      torch::Tensor &X_train, torch::Tensor &y_train) noexcept {
  auto parameters = torch_module.parameters();
  std::vector<at::Tensor> parameter_tensors;
  for (const auto &parameter : parameters) {
    parameter_tensors.push_back(parameter);
  }

  torch::optim::SGD optimizer(parameter_tensors, 0.01);

  torch::nn::MSELoss criterion;

  std::vector<torch::jit::IValue> temp_op;
  temp_op.push_back(X_train);

  // training loop
  for (int64_t epoch = 0; epoch < 500; ++epoch) {
    // Forward pass: Compute predicted y by passing x to the model
    auto y_pred = torch_module.forward(temp_op);

    // convert y_pred to tensor
    auto const &y_pred_tensor = y_pred.toTensor();

    // Compute loss
    auto loss = criterion(y_pred_tensor, y_train);

    // Zero gradients, perform a backward pass, and update the weights.
    optimizer.zero_grad();
    loss.backward();
    optimizer.step();
  }
}

//============================================================================
std::unordered_map<std::string, std::vector<float>> namedParamsToHashMap(
    const torch::jit::named_parameter_list &named_parameters) noexcept {
  std::unordered_map<std::string, std::vector<float>> result;
  for (const auto &named_parameter : named_parameters) {
    auto const &tensor = named_parameter.value;
    auto const &tensor_data = tensor.data_ptr<float>();
    std::vector<float> tensor_vector(tensor_data, tensor_data + tensor.numel());
    result[named_parameter.name] = tensor_vector;
  }
  return result;
}

} // namespace Model

} // namespace Atlas

#endif