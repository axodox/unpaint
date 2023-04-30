#include "pch.h"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace Axodox::Web;
using namespace winrt;
using namespace Windows::Foundation;

int main()
{
  init_apartment();

  HuggingFaceClient huggingFaceClient;
  auto x = huggingFaceClient.GetModels("unpaint,stable_diffusion_model");

  auto y = huggingFaceClient.GetModel("axodoxian/realistic_vision_onnx");

  async_operation operation;
  operation.state_changed(no_revoke, [] (const async_operation_info& info) {
    printf("%s\n", info.status_message.c_str());
  });

  auto z = huggingFaceClient.TryDownloadModel("axodoxian/realistic_vision_onnx", HuggingFaceModelDetails::StableDiffusionOnnxFileset, "D:\\Downloads\\onnx_test", operation);

  Uri uri(L"http://aka.ms/cppwinrt");
  printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());
}
