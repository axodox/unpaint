#include "pch.h"
#include "ControlNetRepository.h"

using namespace Axodox::Web;
using namespace std;
using namespace winrt::Windows::Storage;

namespace winrt::Unpaint
{
  const char* const ControlNetRepository::_controlnetRepository = "axodoxian/controlnet_onnx";

  std::vector<ControlNetModeViewModel> ControlNetRepository::Modes()
  {
    vector<ControlNetModeViewModel> results;

    results.push_back(ControlNetModeViewModel{
      .Id = L"canny",
      .Name = L"Canny edges",
      .Description = L"Generates images based on a monochrome image with white edges on a black background."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"depth",
      .Name = L"Depth image",
      .Description = L"Generates images based on a grayscale image with black representing deep areas and white representing shallow areas."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"hed",
      .Name = L"HED edges",
      .Description = L"Generates images based on a monochrome image with white soft edges on a black background (Holistically-Nested Edge Detection)."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"mlsd",
      .Name = L"MLSD edges",
      .Description = L"Generates images based on a monochrome image composed only of white straight lines on a black background (Mobile Line Segment Detection)."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"normal",
      .Name = L"Normal map",
      .Description = L"Generates images based on a normal map."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"openpose",
      .Name = L"OpenPose",
      .Description = L"Generates images based on an OpenPose bone image."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"scribble",
      .Name = L"Scribble",
      .Description = L"Generates images based on a hand-drawn monochrome image with white outlines on a black background."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"seg",
      .Name = L"Segmentation",
      .Description = L"Generates images based on an ADE20K segmentation protocol image."
      });

    for (auto& result : results)
    {
      result.ExampleInput = format(L"ms-appx:///Assets/controlnet/{}_input.png", result.Id);
      result.ExampleOutput = format(L"ms-appx:///Assets/controlnet/{}_output.png", result.Id);
    }

    return results;
  }

  ControlNetRepository::ControlNetRepository() :
    _root((ApplicationData::Current().LocalCacheFolder().Path() + L"\\controlnet").c_str())
  {
    Refresh();
  }

  const std::vector<std::string>& ControlNetRepository::InstalledModes() const
  {
    return _installedModes;
  }

  bool ControlNetRepository::TryEnsureModes(const std::vector<std::string>& modes, Axodox::Threading::async_operation& operation)
  {
    //Define install & remove tasks
    set<string> filesToInstall{};
    set<string> modesToRemove{ _installedModes.begin(), _installedModes.end() };

    for (auto& mode : modes)
    {
      if (!modesToRemove.erase(mode))
      {
        filesToInstall.emplace(format("controlnet/{}.onnx", mode));
      }
    }

    //Install new modes
    HuggingFaceClient huggingFaceClient{};
    auto result = huggingFaceClient.TryDownloadModel(_controlnetRepository, filesToInstall, _root.parent_path(), operation);

    //Remove old modes
    for (auto& mode : modesToRemove)
    {
      error_code ec;
      filesystem::remove(_root / format("{}.onnx", mode), ec);
    }

    Refresh();

    return result;
  }

  void ControlNetRepository::Refresh()
  {
    vector<string> installedModes;
    for (auto& file : filesystem::directory_iterator{ _root })
    {
      if (file.path().extension() != ".onnx") continue;

      installedModes.push_back(file.path().stem().string());
    }

    _installedModes = installedModes;
  }
}