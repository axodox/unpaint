#pragma once
#include "pch.h"
#include "winrt/Unpaint.h"

namespace winrt::Unpaint
{
  class ControlNetRepository
  {
  public:
    static std::vector<ControlNetModeViewModel> Modes();

    ControlNetRepository();

    const std::filesystem::path& Root() const;
    const std::vector<std::string>& InstalledModes() const;
    const std::vector<std::string>& InstalledAnnotators() const;

    bool TryEnsureModes(const std::vector<std::string>& modes, Axodox::Threading::async_operation& operation);
    void Refresh();

  private:
    static const char* const _controlnetRepository;
    static const std::unordered_map<std::string, std::vector<std::string>> _annotators;
    std::filesystem::path _controlnetRoot, _annotatorRoot;
    std::vector<std::string> _installedModes, _installedAnnotators;
  };
}