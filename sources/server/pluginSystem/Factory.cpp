#include "stdafx.h"
#include "Factory.h"
#include <shared/Log.h>
#include <shared/Executable.h>
#include "Information.h"
#include "Instance.h"
#include "InstanceStateHandler.h"
#include "yPluginApiImplementation.h"
#include "IQualifier.h"
#include "PluginException.hpp"
#include "NativeExecutableCommandLine.h"
#include "Process.h"
#include "Runner.h"
#include "InvalidPluginException.hpp"
#include "Logger.h"
#include "ContextAccessor.h"

namespace pluginSystem
{
   CFactory::CFactory(const boost::filesystem::path& pluginPath)
      : m_pluginPath(pluginPath)
   {
   }

   CFactory::~CFactory()
   {
   }

   boost::shared_ptr<const shared::plugin::information::IInformation> CFactory::createInformation(const std::string& pluginName) const
   {
      return boost::make_shared<CInformation>(getPluginPath(pluginName));
   }

   boost::shared_ptr<IInstance> CFactory::createInstance(boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                         boost::shared_ptr<database::IDataProvider> dataProvider,
                                                         boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer,
                                                         const boost::shared_ptr<IQualifier> qualifier, //TODO faut s'en servir !
                                                         boost::shared_ptr<shared::event::CEventHandler> managerEventHandler) const
   {
      auto pluginInformation = createInformation(instanceData->Type());

      auto instanceStateHandler = createInstanceStateHandler(dataProvider->getPluginRequester(),
                                                             dataAccessLayer->getEventLogger(),
                                                             managerEventHandler,
                                                             instanceData->Id());

      auto logger = createProcessLogger(instanceData->Type());

      auto yPluginApi = createInstanceRunningContext(pluginInformation,
                                                     instanceData,
                                                     dataProvider,
                                                     dataAccessLayer);

      auto commandLine = createCommandLine(pluginInformation,
                                           std::string()/*TODO*/);

      auto runner = createInstanceRunner(commandLine,
                                         logger,
                                         instanceStateHandler);

      return boost::make_shared<CInstance>(pluginInformation,
                                           runner);
   }

   boost::shared_ptr<shared::process::ILogger> CFactory::createProcessLogger(const std::string& pluginName) const
   {
      return boost::make_shared<CLogger>(pluginName);
   }

   boost::shared_ptr<IInstanceStateHandler> CFactory::createInstanceStateHandler(boost::shared_ptr<database::IPluginRequester> dbRequester,
                                                                                 boost::shared_ptr<dataAccessLayer::IEventLogger> eventLogger,
                                                                                 boost::shared_ptr<shared::event::CEventHandler> managerEventHandler,
                                                                                 int instanceId) const
   {
      return boost::make_shared<CInstanceStateHandler>(dbRequester,
                                                       eventLogger,
                                                       managerEventHandler,
                                                       instanceId);
   }

   boost::shared_ptr<ICommandLine> CFactory::createCommandLine(const boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                                                               const std::string& messageQueueId) const
   {
      std::vector<std::string> args;
      args.push_back(messageQueueId);

      return boost::make_shared<CNativeExecutableCommandLine>(pluginInformation->getPath(),
                                                              pluginInformation->getType(),
                                                              args);
   }

   boost::shared_ptr<shared::process::IProcess> CFactory::createProcess(boost::shared_ptr<ICommandLine> commandLine,
                                                                        boost::shared_ptr<shared::process::ILogger> logger,
                                                                        boost::shared_ptr<IInstanceStateHandler> stopNotifier) const
   {
      try
      {
         return boost::make_shared<CProcess>(commandLine, logger);
      }
      catch (CPluginException& e)
      {
         logger->log((boost::format("Error starting plugin %1% : %2%") % commandLine->executable() % e.what()).str());
         stopNotifier->signalStartError(e.what());
         throw;
      }
   }

   boost::shared_ptr<shared::process::IRunner> CFactory::createInstanceRunner(boost::shared_ptr<ICommandLine> commandLine,
                                                                              boost::shared_ptr<shared::process::ILogger> logger,
                                                                              boost::shared_ptr<IInstanceStateHandler> stopNotifier) const
   {
      logger->log("#### START ####");

      auto process = createProcess(commandLine, logger, stopNotifier);
      return boost::make_shared<CRunner>(process, stopNotifier);
   }

   boost::shared_ptr<IContextAccessor> CFactory::createInstanceRunningContext(boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                                                                              boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                                              boost::shared_ptr<database::IDataProvider> dataProvider,
                                                                              boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer) const
   {
      auto apiImplementation(boost::make_shared<CYPluginApiImplementation>(pluginInformation,
                                                                           instanceData,
                                                                           dataProvider,
                                                                           dataAccessLayer->getDeviceManager(),
                                                                           dataAccessLayer->getAcquisitionHistorizer()));

      return boost::make_shared<CContextAccessor>(apiImplementation);
   }


   boost::filesystem::path CFactory::getPluginPath(const std::string& pluginName) const
   {
      auto path(m_pluginPath);
      path /= pluginName;
      return path;
   }

   bool CFactory::isValidPlugin(const boost::filesystem::path& directory) const
   {
      // Check if plugin is a native executable plugin //TODO �a serait mieux dans une classe � part
      const auto expectedLibName(shared::CExecutable::ToFileName(directory.filename().string()));

      for (boost::filesystem::directory_iterator fileIterator(directory); fileIterator != boost::filesystem::directory_iterator(); ++fileIterator)
      {
         if (boost::filesystem::is_regular_file(fileIterator->status()) && // It's a file...
            boost::iequals(fileIterator->path().filename().string(), expectedLibName)) // ...with the same name as sub-directory...
            return true;
      }

      return false;
   }

   std::vector<boost::filesystem::path> CFactory::findPluginDirectories() const
   {
      // Look for all subdirectories in m_pluginPath directory, where it contains library with same name,
      // for example a subdirectory "fakePlugin" containing a "fakePlugin.dll|so" file
      std::vector<boost::filesystem::path> pluginDirectories;

      if (boost::filesystem::exists(m_pluginPath) && boost::filesystem::is_directory(m_pluginPath))
      {
         // Check all subdirectories in m_pluginPath
         for (boost::filesystem::directory_iterator subDirIterator(m_pluginPath); subDirIterator != boost::filesystem::directory_iterator(); ++subDirIterator)
         {
            if (boost::filesystem::is_directory(subDirIterator->status()) && isValidPlugin(subDirIterator->path()))
               pluginDirectories.push_back(subDirIterator->path());
         }
      }

      return pluginDirectories;
   }

   IFactory::AvailablePluginMap CFactory::findAvailablePlugins() const
   {
      AvailablePluginMap availablePlugins;

      auto pluginDirectories = findPluginDirectories();

      for (auto pluginDirectory = pluginDirectories.begin();
         pluginDirectory != pluginDirectories.end(); ++pluginDirectory)
      {
         try
         {
            // Get informations for current found plugin
            auto pluginName = pluginDirectory->filename().string();

            auto pluginInformation = createInformation(pluginName);
            if (pluginInformation->isSupportedOnThisPlatform())
            {
               availablePlugins[pluginName] = pluginInformation;
               YADOMS_LOG(information) << "Plugin " << pluginName << " found";
            }
            else
            {
               YADOMS_LOG(warning) << "Plugin " << pluginName << " found but unsupported on this platform";
            }
         }
         catch (CInvalidPluginException& e)
         {
            // Invalid plugin
            YADOMS_LOG(warning) << e.what() << ", found in plugin path but is not a valid plugin";
         }
         catch (shared::exception::CInvalidParameter& e)
         {
            // Invalid plugin parameter
            YADOMS_LOG(warning) << "Invalid plugin parameter : " << e.what();
         }
         catch (shared::exception::CException & e)
         {
            // Fail to load one plugin
            YADOMS_LOG(warning) << "Invalid plugin : " << e.what();
         }
      }

      return availablePlugins;
   }
} // namespace pluginSystem
