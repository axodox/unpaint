#include "pch.h"
#include "ImportHuggingFaceModelViewModel.h"
#include "ImportHuggingFaceModelViewModel.g.cpp"

using namespace Axodox::MachineLearning::Web;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Unpaint::implementation
{
  hstring ImportHuggingFaceModelViewModel::ModelId()
  {
    return _modelId;
  }

  fire_and_forget ImportHuggingFaceModelViewModel::ModelId(hstring const& value)
  {
    //Update ModelId
    _modelId = value;
    auto modelId = to_string(value);

    if (!modelId.empty())
    {
      //Validate model
      auto lifetime = get_strong();
      apartment_context context;

      co_await resume_background();
      auto model = _huggingFaceClient.GetModel(modelId);
      co_await context;

      //Update IsValid
      _isValid = model && model->IsValidModel(HuggingFaceModelDetails::StableDiffusionOnnxFileset);

      //Update Status
      if (!model) _status = L"Failed to fetch model data.";
      else if (!_isValid) _status = L"The model does not match the Stable Diffusion ONNX schema.";
      else _status = L"";
    }
    else
    {
      _isValid = false;
      _status = L"";
    }
   
    //Update UI
    _propertyChanged(*this, PropertyChangedEventArgs(L"IsValid"));
    _propertyChanged(*this, PropertyChangedEventArgs(L"Status"));
  }

  bool ImportHuggingFaceModelViewModel::IsValid()
  {
    return _isValid;
  }

  hstring ImportHuggingFaceModelViewModel::Status()
  {
    return _status;
  }

  winrt::event_token ImportHuggingFaceModelViewModel::PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
  {
    return _propertyChanged.add(handler);
  }

  void ImportHuggingFaceModelViewModel::PropertyChanged(winrt::event_token const& token) noexcept
  {
    _propertyChanged.remove(token);
  }
}
