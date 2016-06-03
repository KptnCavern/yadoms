#include "stdafx.h"
#include "NativeExecutableCommandLine.h"
#include <shared/Executable.h>

namespace pluginSystem
{
   CNativeExecutableCommandLine::CNativeExecutableCommandLine(bool executableInSystemPath,
                                                              const boost::filesystem::path& workingDirectory,
                                                              const std::string& pluginName,
                                                              const std::vector<std::string> parameters)
      : m_executableInSystemPath(executableInSystemPath),
        m_workingDirectory(workingDirectory),
        m_pluginName(pluginName),
        m_args(parameters)
   {
   }

   CNativeExecutableCommandLine::~CNativeExecutableCommandLine()
   {
   }

   std::string CNativeExecutableCommandLine::executable() const
   {
      return shared::CExecutable::ToFileName(m_pluginName);
   }

   const boost::filesystem::path& CNativeExecutableCommandLine::workingDirectory() const
   {
      return m_workingDirectory;
   }

   bool CNativeExecutableCommandLine::executableInSystemPath() const
   {
      return m_executableInSystemPath;
   }

   const std::vector<std::string>& CNativeExecutableCommandLine::args() const
   {
      return m_args;
   }
} // namespace pluginSystem


