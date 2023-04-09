#pragma once
#include "OnnxEnvironment.h"

namespace Axodox::MachineLearning
{
  void OnnxPrintStatistics(OnnxEnvironment& environment, Ort::Session& session);
}