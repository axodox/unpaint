#pragma once
#include "InferenceView.g.h"
#include "SizeToStringConverter.h"

namespace winrt::Unpaint::implementation
{
  struct InferenceView : InferenceViewT<InferenceView>
  {
    InferenceView() = default;

    InferenceViewModel ViewModel();

  private:
    InferenceViewModel _viewModel;
  };
}

namespace winrt::Unpaint::factory_implementation
{
  struct InferenceView : InferenceViewT<InferenceView, implementation::InferenceView>
  {
  };
}
