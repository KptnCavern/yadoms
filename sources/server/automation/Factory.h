#pragma once
#include "IFactory.h"
#include "InstanceStateHandler.h"
#include <shared/process/IProcess.h>
#include <shared/process/ILogger.h>
#include <shared/process/ICommandLine.h>
#include "IIpcAdapter.h"
#include <IPathProvider.h>

namespace automation
{
   namespace script
   {
      //--------------------------------------------------------------
      /// \brief	IFactory implementation
      //--------------------------------------------------------------
      class CFactory : public IFactory
      {
      public:
         //--------------------------------------------------------------
         /// \brief	Constructor
         /// \param[in] pathProvider      The path provider
         //--------------------------------------------------------------
         CFactory(const IPathProvider& pathProvider);

         //--------------------------------------------------------------
         /// \brief	Destructor
         //--------------------------------------------------------------
         virtual ~CFactory();

         // IFactory Implementation
         AvailableInterpretersMap findAvailableInterpreters() const override;
         boost::shared_ptr<IInstance> createInstance(boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                     boost::shared_ptr<database::IDataProvider> dataProvider,
                                                     boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer,
                                                     boost::shared_ptr<IInstanceStoppedListener> instanceStoppedListener) const override;
         boost::filesystem::path interpreterDataPath(int instanceId) const override;
         // [END] IFactory Implementation


      private:
         boost::shared_ptr<IInstance> createInternalPluginInstance(boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                                   boost::shared_ptr<database::IDataProvider> dataProvider,
                                                                   boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer,
                                                                   boost::shared_ptr<IQualifier> qualifier,
                                                                   boost::shared_ptr<IInstanceStoppedListener> instanceStoppedListener) const;

         boost::shared_ptr<const shared::plugin::information::IInformation> createInformation(const std::string& pluginName) const;


         boost::shared_ptr<shared::process::ILogger> createProcessLogger(boost::shared_ptr<const database::entities::CPlugin> instanceData) const;

         boost::shared_ptr<shared::process::ICommandLine> createCommandLine(const boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                                                                            const std::string& messageQueueId) const;

         boost::shared_ptr<CInstanceStateHandler> createInstanceStateHandler(boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                                             boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                                                                             boost::shared_ptr<database::IDataProvider> dataProvider,
                                                                             boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer,
                                                                             boost::shared_ptr<IQualifier> qualifier,
                                                                             boost::shared_ptr<IInstanceStoppedListener> instanceStoppedListener) const;

         boost::shared_ptr<shared::process::IProcess> createInstanceProcess(boost::shared_ptr<shared::process::ICommandLine> commandLine,
                                                                            boost::shared_ptr<shared::process::ILogger> logger,
                                                                            boost::shared_ptr<CInstanceStateHandler> instanceStatteHandler) const;

         boost::shared_ptr<CYPluginApiImplementation> createApiPluginImplementation(boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                                                                                    boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                                                    boost::shared_ptr<IInstanceStateHandler> instanceStateHandler,
                                                                                    boost::shared_ptr<database::IDataProvider> dataProvider,
                                                                                    boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer) const;

         boost::shared_ptr<IIpcAdapter> createInstanceRunningContext(boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                                                                     boost::shared_ptr<const database::entities::CPlugin> instanceData,
                                                                     boost::shared_ptr<IInstanceStateHandler> instanceStateHandler,
                                                                     boost::shared_ptr<database::IDataProvider> dataProvider,
                                                                     boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer) const;

         bool isValidPlugin(const boost::filesystem::path& directory) const;
         std::vector<boost::filesystem::path> findPluginDirectories() const;


         const IPathProvider& m_pathProvider;
         const boost::shared_ptr<const shared::ILocation> m_locationProvider;
      };
   }
} // namespace automation::script


