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

    const std::vector<std::string>& InstalledModes() const;

    bool TryEnsureModes(const std::vector<std::string>& modes, Axodox::Threading::async_operation& operation);
    void Refresh();

  private:
    static const char* const _controlnetRepository;
    std::filesystem::path _root;
    std::vector<std::string> _installedModes;
  };
}