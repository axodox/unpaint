#include "pch.h"
#include "ModelsViewModel.h"
#include "ModelsViewModel.g.cpp"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Web;

namespace winrt::Unpaint::implementation
{
  const char* const ModelsViewModel::_modelFilter = "unpaint,stable_diffusion_model";

  ModelsViewModel::ModelsViewModel() :
    _availableModels(single_threaded_observable_vector<ModelViewModel>()),
    _installedModels(single_threaded_observable_vector<ModelViewModel>())
  {
    UpdateAvailableModelsAsync();
  }

  Windows::Foundation::Collections::IObservableVector<ModelViewModel> ModelsViewModel::AvailableModels()
  {
    return _availableModels;
  }

  fire_and_forget ModelsViewModel::UpdateAvailableModelsAsync()
  {
    apartment_context callerContext;
    
    auto lifetime = get_strong();
    co_await resume_background();

    HuggingFaceClient client{};
    auto models = client.GetModels(_modelFilter);

    co_await callerContext;

    _availableModels.Clear();
    for (auto& model : models)
    {
      ModelViewModel viewModel{
        .Id = to_hstring(model)
      };

      _availableModels.Append(viewModel);
    }
  }

  Windows::Foundation::Collections::IObservableVector<ModelViewModel> ModelsViewModel::InstalledModels()
  {
    return _installedModels;
  }
}
