#include "stdafx.h"
#include "../../FileSystem.h"

namespace tools
{
   Poco::Path CFileSystem::getLogFolder()
   {
      // No central log folder on Windows system
      static const Poco::Path emptyPath;
      return emptyPath;
   }
} // namespace tools


