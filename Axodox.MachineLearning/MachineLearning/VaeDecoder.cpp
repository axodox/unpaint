#include "pch.h"
#include "VaeDecoder.h"

using namespace Ort;
using namespace std;

namespace Axodox::MachineLearning
{
  VaeDecoder::VaeDecoder(OnnxEnvironment& environment) :
    _environment(environment),
    _session(nullptr)
  {
    _session = { _environment.Environment(), (_environment.RootPath() / L"vae_decoder/model.onnx").c_str(), _environment.DefaultSessionOptions() };
  }

  Tensor VaeDecoder::DecodeVae(Tensor text)
  {
    //Load inputs
    auto inputValue = text.ToOrtValue(_environment.MemoryInfo());

    //Bind values
    IoBinding bindings{ _session };
    bindings.BindInput("latent_sample", inputValue);
    bindings.BindOutput("sample", _environment.MemoryInfo());

    //Run inference
    _session.Run({}, bindings);

    //Get result
    auto outputValues = bindings.GetOutputValues();
    return Tensor::FromOrtValue(outputValues[0]);
  }
}