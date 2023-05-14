#include "pch.h"
#ifdef USE_ONNX
#include "VaeEncoder.h"

using namespace Ort;
using namespace std;

namespace Axodox::MachineLearning
{
  VaeEncoder::VaeEncoder(OnnxEnvironment& environment) :
    _environment(environment),
    _session(nullptr)
  {
    _session = { _environment.Environment(), (_environment.RootPath() / L"vae_encoder/model.onnx").c_str(), _environment.DefaultSessionOptions() };
  }

  Tensor VaeEncoder::EncodeVae(const Tensor& image)
  {
    //Load inputs
    auto inputValues = image.Split(image.Shape[0]);

    Tensor results;
    for (size_t i = 0; i < image.Shape[0]; i++)
    {
      //Bind values
      IoBinding bindings{ _session };
      bindings.BindInput("sample", inputValues[i].ToOrtValue(_environment.MemoryInfo()));
      bindings.BindOutput("latent_sample", _environment.MemoryInfo());

      //Run inference
      _session.Run({}, bindings);

      //Get result
      auto outputValues = bindings.GetOutputValues();
      auto result = Tensor::FromOrtValue(outputValues[0]);

      if (!results.IsValid())
      {
        auto shape = result.Shape;
        shape[0] = image.Shape[0];
        results = { result.Type, shape };
      }

      memcpy(results.AsPointer<float>(i), result.AsPointer<float>(), result.ByteCount());
    }

    return results;
  }
}
#endif