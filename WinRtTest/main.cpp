#include "pch.h"
#include "Web/HuggingFaceClient.h"

using namespace Axodox::Web;
using namespace winrt;
using namespace Windows::Foundation;

int main()
{
  init_apartment();

  HuggingFaceClient huggingFaceClient;
  auto x = huggingFaceClient.GetModels();

  auto y = huggingFaceClient.GetModel("axodoxian/realistic_vision_onnx");

  Uri uri(L"http://aka.ms/cppwinrt");
  printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());
}
