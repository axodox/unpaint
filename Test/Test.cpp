// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "UI/Window.h"
#include "Infrastructure/Events.h"
#include "Graphics/Swap Chains/HwndSwapChain.h"
#include "Storage/FileStream.h"
#include "Storage/FileIO.h"

#include "MachineLearning/TextTokenizer.h"
#include "MachineLearning/TextEncoder.h"
#include "MachineLearning/StableDiffustionInferer.h"
#include "MachineLearning/VaeDecoder.h"
#include "MachineLearning/VaeEncoder.h"
#include "MachineLearning/TextEmbedder.h"

using namespace Axodox::Graphics;
using namespace Axodox::Infrastructure;
using namespace Axodox::UI;
using namespace Axodox::Storage;
using namespace Axodox::MachineLearning;
using namespace std;
using namespace winrt;


TextureData ReadPfm(const std::filesystem::path& path)
{
  //Create stream
  file_stream stream{ path, file_mode::read };

  //Read header
  auto magic = stream.read_line();
  auto resolution = stream.read_line();
  uint32_t width, height;
  sscanf_s(resolution.c_str(), "%u %u", &width, &height);

  auto bias = stream.read_line();

  //Read depth data
  TextureData result{ width, height, DXGI_FORMAT_R32_FLOAT };
  stream.read(result.Buffer);

  return result;
}

void test()
{
  std::cout << "Hello World!\n";

  auto textureData = TextureData::FromBuffer(read_file(L"C:\\dev\\ai\\MiDaS\\input\\test.png"));
  auto depthData = ReadPfm(L"C:\\dev\\ai\\MiDaS\\output\\test-dpt_beit_large_512.pfm");

  auto depthBuffer = depthData.Cast<float>();

  Window window{ L"Hello world!" };

  GraphicsDevice device{};
  HwndSwapChain swapChain{ device, *window };

  window.Painting(no_revoke, [&](Window*) {
    swapChain.BackBuffer()->Clear();
    swapChain.Present();
    });

  window.Resized(no_revoke, [&](Window*) {
    swapChain.Resize();
    });

  window.Show();
  window.Run();
}

int main()
{
  winrt::init_apartment();

  OnnxEnvironment onnxEnvironment{ L"C:/dev/ai/realistic_vision_v1.4-fp16-vram" };

  TextEmbedder embedder{onnxEnvironment};
  embedder.ProcessText("a (simple (not so much:15) :0.1) prompt");

  //OnnxEnvironment onnxEnvironment{ L"D:/dev/Stable-Diffusion-USE_ONNX-FP16/model/safetensors-sd15-fp16" };
  //OnnxEnvironment onnxEnvironment{ L"D:/dev/Stable-Diffusion-USE_ONNX-FP16/model/safetensors-protogenX53Photorealism_10-fp16" };
  //OnnxEnvironment onnxEnvironment{ L"C:/dev/StableDiffusion/StableDiffusion" };
  //OnnxEnvironment onnxEnvironment{ L"D:/dev/stable-diffusion-2-1-fp16-onnx" };

  //Load source image
  Tensor latentInput;
  {
    auto pngBuffer = read_file(L"bin/input.png");
    auto imageTexture = TextureData::FromBuffer(pngBuffer);
    Tensor imageTensor = Tensor::FromTextureData(imageTexture);

    VaeEncoder vaeEncoder{ onnxEnvironment };
    latentInput = vaeEncoder.EncodeVae(imageTensor);
  }

  //Load mask image
  Tensor maskInput;
  {
    auto pngBuffer = read_file(L"bin/mask.png");
    auto imageTexture = TextureData::FromBuffer(pngBuffer, TextureImageFormat::Gray8);
    maskInput = Tensor::FromTextureData(imageTexture);
  }

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

  //Run stable diffusion
  Tensor latentResult;
  {
    StableDiffusionInferer stableDiffusion{ onnxEnvironment };

    StableDiffusionOptions options{
      .StepCount = 20,
      .BatchSize = 1,
      .Width = 768,
      .Height = 768,
      .Seed = 88,
      .TextEmbeddings = textEmbeddings,
      .LatentInput = latentInput,
      .MaskInput = maskInput,
      .DenoisingStrength = 0.6f
    };

    latentResult = stableDiffusion.RunInference(options);
  }

  //Decode VAE
  {
    VaeDecoder vaeDecoder{ onnxEnvironment };
    auto imageTensor = vaeDecoder.DecodeVae(latentResult);

    auto imageTextures = imageTensor.ToTextureData();

    for (auto i = 0; auto & imageTexture : imageTextures)
    {
      auto pngBuffer = imageTexture.ToBuffer();
      write_file(std::format(L"bin/test{}.png", i++), pngBuffer);
    }
  }

  //Done
  printf("done.");

  return 0;
}