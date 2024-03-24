#include "pch.h"
#include "ControlNetRepository.h"

using namespace Axodox::MachineLearning::Web;
using namespace std;
using namespace winrt::Windows::Storage;

namespace winrt::Unpaint
{
  const char* const ControlNetRepository::_controlnetRepository = "axodoxian/controlnet_onnx";

  const std::unordered_map<std::string, std::vector<std::string>> ControlNetRepository::_annotators = {
    { "canny", { "canny.onnx" } },
    { "depth", { "depth.onnx" } },
    { "hed", { "hed.onnx" } },
    { "openpose", { "openpose.onnx" } }
  };

  std::vector<ControlNetModeViewModel> ControlNetRepository::Modes()
  {
    vector<ControlNetModeViewModel> results;

    results.push_back(ControlNetModeViewModel{
      .Id = L"canny",
      .Name = L"Canny edges",
      .ShortName = L"Canny",
      .Description = L"Generates images based on a monochrome image with white edges on a black background."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"depth",
      .Name = L"Depth image",
      .ShortName = L"Depth",
      .Description = L"Generates images based on a grayscale image with black representing deep areas and white representing shallow areas."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"hed",
      .Name = L"HED edges",
      .ShortName = L"HED",
      .Description = L"Generates images based on a monochrome image with white soft edges on a black background (Holistically-Nested Edge Detection)."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"mlsd",
      .Name = L"MLSD edges",
      .ShortName = L"MLSD",
      .Description = L"Generates images based on a monochrome image composed only of white straight lines on a black background (Mobile Line Segment Detection)."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"normal",
      .Name = L"Normal map",
      .ShortName = L"Normal",
      .Description = L"Generates images based on a normal map."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"openpose",
      .Name = L"OpenPose",
      .ShortName = L"Pose",
      .Description = L"Generates images based on an OpenPose bone image."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"scribble",
      .Name = L"Scribble",
      .ShortName = L"Scribble",
      .Description = L"Generates images based on a hand-drawn monochrome image with white outlines on a black background."
      });

    results.push_back(ControlNetModeViewModel{
      .Id = L"seg",
      .Name = L"Segmentation",
      .ShortName = L"Seg",
      .Description = L"Generates images based on an ADE20K segmentation protocol image."
      });

    results.push_back(ControlNetModeViewModel{
     .Id = L"inpaint",
     .Name = L"Inpainting",
     .ShortName = L"Inpaint",
     .Description = L"Generates images based on an existing image."
      });

    for (auto& result : results)
    {
      result.ExampleInput = format(L"ms-appx:///Assets/controlnet/{}_input.png", result.Id);
      result.ExampleOutput = format(L"ms-appx:///Assets/controlnet/{}_output.png", result.Id);
    }

    return results;
  }

  ControlNetRepository::ControlNetRepository() :
    _controlnetRoot((ApplicationData::Current().LocalCacheFolder().Path() + L"\\controlnet").c_str()),
    _annotatorRoot((ApplicationData::Current().LocalCacheFolder().Path() + L"\\annotators").c_str())
  {
    std::error_code ec;
    filesystem::create_directories(_controlnetRoot, ec);
    filesystem::create_directories(_annotatorRoot, ec);

    Refresh();
  }

  const std::filesystem::path& ControlNetRepository::Root() const
  {
    return _controlnetRoot;
  }

  const std::vector<std::string>& ControlNetRepository::InstalledModes() const
  {
    return _installedModes;
  }

  const std::vector<std::string>& ControlNetRepository::InstalledAnnotators() const
  {
    return _installedAnnotators;
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

        auto annotatorIt = _annotators.find(mode);
        if (annotatorIt != _annotators.end())
        {
          for (auto& annotator : annotatorIt->second)
          {
            filesToInstall.emplace("annotators/" + annotator);
          }
        }
      }
    }

    //Install new modes
    HuggingFaceClient huggingFaceClient{};
    auto result = huggingFaceClient.TryDownloadModel(_controlnetRepository, filesToInstall, {}, _controlnetRoot.parent_path(), operation);

    //Remove old modes
    for (auto& mode : modesToRemove)
    {
      error_code ec;
      filesystem::remove(_controlnetRoot / format("{}.onnx", mode), ec);

      auto annotatorIt = _annotators.find(mode);
      if (annotatorIt != _annotators.end())
      {
        for (auto& annotator : annotatorIt->second)
        {
          filesystem::remove(_annotatorRoot / annotator, ec);
        }
      }
    }

    Refresh();

    return result;
  }

  void ControlNetRepository::Refresh()
  {
    //Detect controlnet modes
    vector<string> installedModes;
    for (auto& file : filesystem::directory_iterator{ _controlnetRoot })
    {
      if (file.path().extension() != ".onnx") continue;

      installedModes.push_back(file.path().stem().string());
    }

    //Detect annotators
    error_code ec;
    vector<string> installedAnnotators;
    for (auto& mode : installedModes)
    {
      bool isInstalled = false;

      auto annotatorIt = _annotators.find(mode);
      if (annotatorIt != _annotators.end())
      {
        isInstalled = true;
        for (auto& annotator : annotatorIt->second)
        {
          if (!filesystem::exists(_annotatorRoot / annotator, ec))
          {
            isInstalled = false;
            break;
          }
        }
      }

      if (isInstalled) installedAnnotators.push_back(mode);
    }

    //Update state
    _installedModes = installedModes;
    _installedAnnotators = installedAnnotators;
  }
}