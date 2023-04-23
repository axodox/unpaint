#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

#include "MachineLearning/TextTokenizer.h"
#include "MachineLearning/TextEncoder.h"
#include "MachineLearning/StableDiffustionInferer.h"
#include "MachineLearning/VaeDecoder.h"
#include "MachineLearning/VaeEncoder.h"

#include "Storage/FileIO.h"

using namespace Axodox::MachineLearning;
using namespace Axodox::Storage;


using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::Axodox_UniversalDiffusion::implementation
{
  int32_t MainPage::MyProperty()
  {
    throw hresult_not_implemented();
  }

  void MainPage::MyProperty(int32_t /* value */)
  {
    throw hresult_not_implemented();
  }

  void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
  {
    OnnxEnvironment onnxEnvironment{ app_folder() / L"model" };

    //Create text embeddings
    Tensor textEmbeddings;
    {
      //Encode text
      TextTokenizer textTokenizer{ onnxEnvironment };
      TextEncoder textEncoder{ onnxEnvironment };

      auto tokenizedNegativePrompt = textTokenizer.TokenizeText("blurry, render");//textTokenizer.GetUnconditionalTokens();
      auto encodedNegativePrompt = textEncoder.EncodeText(tokenizedNegativePrompt);

      auto tokenizedPositivePrompt = textTokenizer.TokenizeText("a stag standing in a snowy winter forest at noon, closeup");
      auto encodedPositivePrompt = textEncoder.EncodeText(tokenizedPositivePrompt);

      textEmbeddings = encodedNegativePrompt.Concat(encodedPositivePrompt);
    }

    myButton().Content(box_value(L"Clicked"));
  }
}
