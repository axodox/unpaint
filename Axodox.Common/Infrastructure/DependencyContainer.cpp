#include "pch.h"
#include "DependencyContainer.h"

namespace Axodox::Infrastructure
{
#ifdef USE_GLOBAL_DEPENDENCIES
  dependency_container dependencies;
#endif // USE_GLOBAL_DEPENDENCIES
}