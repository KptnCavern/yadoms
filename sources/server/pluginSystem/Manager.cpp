#include "stdafx.h"

#include "Manager.h"
#ifdef _DEBUG
#include "BasicQualifier.h"
#else
#include "IndicatorQualifier.h"
#endif
#include <shared/exception/InvalidParameter.hpp>
#include <shared/exception/EmptyResult.hpp>
#include <shared/plugin/yPluginApi/historization/PluginState.h>

#include "Factory.h"

#include <shared/Log.h>
#include "PluginException.hpp"
#include "InvalidPluginException.hpp"
#include "InstanceRemoverRaii.hpp"

namespace pluginSystem
{
   CManager::CManager(const IPathProvider& pathProvider,
                      boost::shared_ptr<database::IDataProvider> dataProvider,
                      boost::shared_ptr<dataAccessLayer::IDataAccessLayer> dataAccessLayer)
      :
      m_factory(boost::make_shared<CFactory>(pathProvider)),
      m_dataProvider(dataProvider),
      m_pluginDBTable(dataProvider->getPluginRequester()),
#ifdef _DEBUG
      m_qualifier(boost::make_shared<CBasicQualifier>(dataProvider->getPluginEventLoggerRequester(), dataAccessLayer->getEventLogger())),
#else
      m_qualifier(boost::make_shared<CIndicatorQualifier>(dataProvider->getPluginEventLoggerRequester(), dataAccessLayer->getEventLogger())),
#endif
      m_dataAccessLayer(dataAccessLayer),
      m_instanceRemover(boost::make_shared<CInstanceRemover>(m_runningInstancesMutex, m_runningInstances))
   {
   }

   CManager::~CManager()
   {
      stop();
   }

   void CManager::start()
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      // Initialize the plugin list (detect available plugins)
      updatePluginList();

      startAllInstances();
   }

   void CManager::stop()
   {
      stopInstances();
   }

   void CManager::startAllInstances()
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      if (!m_runningInstances.empty())
         throw shared::exception::CException("Some plugins are already started, are you sure that manager was successfuly stopped ?");

      if (!startInstances(getInstanceList()))
         YADOMS_LOG(error) << "One or more plugins failed to start, check plugins page for details";

      //start the internal plugin
      startInternalPlugin();
   }

   bool CManager::startInstances(const std::vector<boost::shared_ptr<database::entities::CPlugin> >& instances)
   {
      auto allInstancesStarted = true;
      for (auto it = instances.begin(); it != instances.end(); ++it)
      {
         try
         {
            if ((*it)->Category() != database::entities::EPluginCategory::kSystem)
               if ((*it)->AutoStart())
                  startInstance((*it)->Id());
         }
         catch (CPluginException& ex)
         {
            YADOMS_LOG(error) << "Unable to start plugin " << (*it)->DisplayName() << "(" << ex.what() << "), skipped";
            allInstancesStarted = false;
         }
      }

      return allInstancesStarted;
   }

   void CManager::updatePluginList()
   {
      // Plugin directory have changed, so re-build available plugins list

      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      boost::lock_guard<boost::recursive_mutex> lock2(m_availablePluginsMutex);
      m_availablePlugins = m_factory->findAvailablePlugins();
   }


   IFactory::AvailablePluginMap CManager::getPluginList() const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_availablePluginsMutex);
      return m_availablePlugins;
   }

   int CManager::getPluginQualityIndicator(const std::string& pluginName) const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_availablePluginsMutex);

      if (m_availablePlugins.find(pluginName) == m_availablePlugins.end())
         throw CInvalidPluginException(pluginName); // Invalid plugin

      return m_qualifier->getQualityLevel(m_availablePlugins.at(pluginName));
   }

   int CManager::createInstance(const database::entities::CPlugin& data)
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      // First step, record instance in database, to get its ID
      auto instanceId = m_pluginDBTable->addInstance(data);

      // Next create instance
      startInstance(instanceId);

      return instanceId;
   }

   void CManager::deleteInstance(int id)
   {
      try
      {
         auto instanceData = m_pluginDBTable->getInstance(id);
         if (instanceData->Category() == database::entities::EPluginCategory::kSystem)
            return;

         // Stop plugin instance
         stopInstanceAndWaitForStopped(id);

         // Remove in database
         boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
         m_dataProvider->getDeviceRequester()->removeAllDeviceForPlugin(id);
         m_pluginDBTable->removeInstance(id);
      }
      catch (shared::exception::CException& e)
      {
         YADOMS_LOG(error) << "Unable to delete plugin (" << id << ") : " << e.what();
         throw shared::exception::CInvalidParameter(boost::lexical_cast<std::string>(id));
      }
   }

   std::vector<boost::shared_ptr<database::entities::CPlugin> > CManager::getInstanceList() const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      return m_pluginDBTable->getInstances();
   }

   boost::shared_ptr<database::entities::CPlugin> CManager::getInstance(int id) const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      return m_pluginDBTable->getInstance(id);
   }

   boost::shared_ptr<IInstance> CManager::getRunningInstance(int id) const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      auto instance = m_runningInstances.find(id);
      if (instance == m_runningInstances.end())
         throw CPluginException((boost::format("Instance #%1% is not running") % id).str());

      return instance->second;
   }

   void CManager::updateInstance(const database::entities::CPlugin& newData)
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      // Check for supported modifications
      if (!newData.Id.isDefined())
         throw shared::exception::CException("Update instance : instance ID was not provided");

      // First get old configuration from database
      boost::shared_ptr<const database::entities::CPlugin> previousData = m_pluginDBTable->getInstance(newData.Id());

      // Next, update configuration in database
      m_pluginDBTable->updateInstance(newData);

      // Last, apply modifications
      if (newData.Configuration.isDefined()
         && previousData->Configuration() != newData.Configuration()) // No need to notify configuration if instance was just enabled/disabled
      {
         // Configuration was updated, notify the instance if running
         if (m_runningInstances.find(newData.Id()) != m_runningInstances.end())
            m_runningInstances[newData.Id()]->updateConfiguration(newData.Configuration());
      }
   }

   void CManager::startAllInstancesOfPlugin(const std::string& pluginName)
   {
      // Find instances to start
      std::vector<int> instancesToStart;
      std::vector<boost::shared_ptr<database::entities::CPlugin> > allInstances = getInstanceList();
      for (std::vector<boost::shared_ptr<database::entities::CPlugin> >::const_iterator instance = allInstances.begin(); instance != allInstances.end(); ++instance)
      {
         if (boost::iequals((*instance)->Type(), pluginName) && (*instance)->AutoStart())
            instancesToStart.push_back((*instance)->Id());
      }

      // Start all instances of this plugin
      for (std::vector<int>::const_iterator instanceToStart = instancesToStart.begin(); instanceToStart != instancesToStart.end(); ++instanceToStart)
         startInstance(*instanceToStart);
   }

   void CManager::stopAllInstancesOfPlugin(const std::string& pluginName)
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      // Find instances to stop
      std::vector<int> instancesToStop;
      for (PluginInstanceMap::const_iterator instance = m_runningInstances.begin(); instance != m_runningInstances.end(); ++instance)
      {
         if (instance->second && boost::iequals(instance->second->aboutPlugin()->getType(), pluginName))
            instancesToStop.push_back(instance->first);
      }

      // Stop all instances of this plugin
      for (std::vector<int>::const_iterator instanceToStop = instancesToStop.begin(); instanceToStop != instancesToStop.end(); ++instanceToStop)
         requestStopInstance(*instanceToStop);
   }

   void CManager::startInstance(int id)
   {
      try
      {
         auto instanceData(getInstance(id));

         if (isInstanceRunning(instanceData->Id()))
            return; // Already started ==> nothing more to do

         YADOMS_LOG(information) << "Start plugin instance " << instanceData->DisplayName();

         // Create instance
         boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
         m_runningInstances[instanceData->Id()] = m_factory->createInstance(instanceData,
                                                                            m_dataProvider,
                                                                            m_dataAccessLayer,
                                                                            m_qualifier,
                                                                            m_instanceRemover);
      }
      catch (shared::exception::CEmptyResult& e)
      {
         throw CPluginException((boost::format("Invalid plugin instance %1%, element not found in database : %2%") % id % e.what()).str());
      }
      catch (CInvalidPluginException& e)
      {
         throw CPluginException((boost::format("Invalid plugin instance %1%, because of an invalid plugin : %2%") % id % e.what()).str());
      }
      catch (shared::exception::COutOfRange& e)
      {
         throw CPluginException((boost::format("Invalid plugin instance %1% configuration, out of range : %2%") % id % e.what()).str());
      }
      catch (std::exception& e)
      {
         throw CPluginException((boost::format("Unable to start instance %1% %2%") % id % e.what()).str());
      }
      catch (...)
      {
         throw CPluginException((boost::format("Unable to start instance %1%, unknown error") % id).str());
      }
   }

   void CManager::stopInstances()
   {
      YADOMS_LOG(information) << "pluginSystem::CManager stop plugins...";

      stopInternalPlugin();

      // Mutex must be released to process instance stop, so use a temporary vector of instances to stop
      std::vector<int> idsToStop;
      {
         boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
         for (auto runningInstance = m_runningInstances.begin(); runningInstance != m_runningInstances.end(); ++runningInstance)
            idsToStop.push_back(runningInstance->first);
      }

      boost::thread_group threads;
      for (auto idToStop = idsToStop.begin(); idToStop != idsToStop.end(); ++idToStop)
         threads.create_thread(boost::bind(&CManager::stopInstanceAndWaitForStoppedThreaded, this, *idToStop));
      threads.join_all();

      YADOMS_LOG(information) << "pluginSystem::CManager all plugins are stopped";
   }

   void CManager::requestStopInstance(int id)
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      auto instance = m_runningInstances.find(id);

      if (instance == m_runningInstances.end())
         return; // Already stopped ==> nothing more to do

      instance->second->requestStop();
   }

   void CManager::killInstance(int id)
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);

      auto instance = m_runningInstances.find(id);

      if (instance == m_runningInstances.end())
         return; // Already stopped ==> nothing more to do

      instance->second->kill();
   }

   void CManager::stopInstanceAndWaitForStopped(int id)
   {
      if (!isInstanceRunning(id))
         return;

      CInstanceRemoverRaii instanceRemover(m_instanceRemover, id);

      requestStopInstance(id);
      if (instanceRemover.eventHandler()->waitForEvents(boost::posix_time::seconds(20)) != shared::event::kTimeout)
         return;

      YADOMS_LOG(warning) << "pluginSystem::CManager, instance #" << id << " didn't stop when requested. Will be killed.";
      killInstance(id);

      if (instanceRemover.eventHandler()->waitForEvents(boost::posix_time::seconds(20)) != shared::event::kTimeout)
         return;

      throw CPluginException((boost::format("pluginSystem::CManager, unable to stop instance #%1%") % id).str());
   }

   void CManager::stopInstanceAndWaitForStoppedThreaded(int id)
   {
      try
      {
         stopInstanceAndWaitForStopped(id);
      }
      catch (CPluginException& exception)
      {
         YADOMS_LOG(error) << exception.what();
      }
   }

   void CManager::startInternalPlugin()
   {
      startInstance(m_pluginDBTable->getSystemInstance()->Id());
   }

   void CManager::stopInternalPlugin()
   {
      stopInstanceAndWaitForStopped(m_pluginDBTable->getSystemInstance()->Id());
   }

   bool CManager::isInstanceRunning(int id) const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      return m_runningInstances.find(id) != m_runningInstances.end();
   }

   shared::CDataContainer CManager::getInstanceFullState(int id) const
   {
      if (!isInstanceRunning(id))
      {
         // Instance is not running, so can be in error or stopped state
         boost::shared_ptr<database::entities::CDevice> device;
         try
         {
            // First find the pluginState device associated with the plugin
            device = m_dataProvider->getDeviceRequester()->getDevice(id, "pluginState");
         }
         catch (shared::exception::CEmptyResult&)
         {
            // Device doesn't exist, probably not supported by plugin. Plugin is then considered as stopped.
            shared::CDataContainer defaultState;
            defaultState.set("state", shared::plugin::yPluginApi::historization::EPluginState::kStopped);
            return defaultState;
         }

         try
         {
            auto stateKw = m_dataProvider->getKeywordRequester()->getKeyword(device->Id, "state");
            shared::plugin::yPluginApi::historization::EPluginState state(m_dataProvider->getAcquisitionRequester()->getKeywordLastData(stateKw->Id)->Value());
            if (state == shared::plugin::yPluginApi::historization::EPluginState::kError)
            {
               // In error state
               auto customMessageIdKw = m_dataProvider->getKeywordRequester()->getKeyword(device->Id, "customMessageId");
               shared::CDataContainer defaultState;
               defaultState.set("state", state);
               defaultState.set("messageId", m_dataProvider->getAcquisitionRequester()->getKeywordLastData(customMessageIdKw->Id)->Value());
               return defaultState;
            }

            // Normaly stopped
            shared::CDataContainer defaultState;
            defaultState.set("state", shared::plugin::yPluginApi::historization::EPluginState::kStopped);
            return defaultState;
         }
         catch (shared::exception::CEmptyResult&)
         {
            // pluginState keyword exist, but was never historized, so considered as stopped.
            shared::CDataContainer defaultState;
            defaultState.set("state", shared::plugin::yPluginApi::historization::EPluginState::kStopped);
            return defaultState;
         }
      }

      // Instance is running
      boost::shared_ptr<database::entities::CDevice> device;
      try
      {
         // First find the pluginState device associated with the plugin
         device = m_dataProvider->getDeviceRequester()->getDevice(id, "pluginState");
      }
      catch (shared::exception::CEmptyResult&)
      {
         // Device doesn't exist, probably not supported by plugin. Plugin is then considered as running.
         shared::CDataContainer defaultState;
         defaultState.set("state", shared::plugin::yPluginApi::historization::EPluginState::kRunning);
         return defaultState;
      }

      try
      {
         auto stateKw = m_dataProvider->getKeywordRequester()->getKeyword(device->Id, "state");
         auto customMessageIdKw = m_dataProvider->getKeywordRequester()->getKeyword(device->Id, "customMessageId");
         shared::CDataContainer defaultState;
         defaultState.set("state", m_dataProvider->getAcquisitionRequester()->getKeywordLastData(stateKw->Id)->Value());
         defaultState.set("messageId", m_dataProvider->getAcquisitionRequester()->getKeywordLastData(customMessageIdKw->Id)->Value());
         return defaultState;
      }
      catch (shared::exception::CEmptyResult&)
      {
         // pluginState keyword exist, but was never historized, so considered as unknown.
         shared::CDataContainer defaultState;
         defaultState.set("state", shared::plugin::yPluginApi::historization::EPluginState::kUnknown);
         return defaultState;
      }
   }

   shared::plugin::yPluginApi::historization::EPluginState CManager::getInstanceState(int id) const
   {
      auto fullState = getInstanceFullState(id);
      return fullState.get<shared::plugin::yPluginApi::historization::EPluginState>("state");
   }

   void CManager::postCommand(int id, boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> command) const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      auto instance(getRunningInstance(id));

      YADOMS_LOG(debug) << "Send command " << shared::plugin::yPluginApi::IDeviceCommand::toString(command) << " to plugin " << instance->about()->DisplayName();

      instance->postDeviceCommand(command);
   }

   void CManager::postExtraCommand(int id, boost::shared_ptr<const shared::plugin::yPluginApi::IExtraCommand> command) const
   {
      boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
      auto instance(getRunningInstance(id));

      YADOMS_LOG(debug) << "Send extra command " << command->getCommand() << " to plugin " << instance->about()->DisplayName();

      instance->postExtraCommand(command);
   }

   void CManager::postManuallyDeviceCreationRequest(int id, boost::shared_ptr<shared::plugin::yPluginApi::IManuallyDeviceCreationRequest>& request) const
   {
      try
      {
         boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
         auto instance = getRunningInstance(id);

         YADOMS_LOG(debug) << "Send manually device (" << request->getData().getDeviceName() << ") creation request " << " to plugin " << instance->about()->DisplayName();

         instance->postManuallyDeviceCreationRequest(request);
      }
      catch (CPluginException& e)
      {
         request->sendError((boost::format("Error when requesting binding query %1%") % e.what()).str());
      }
   }

   void CManager::postBindingQueryRequest(int id, boost::shared_ptr<shared::plugin::yPluginApi::IBindingQueryRequest>& request)
   {
      try
      {
         boost::lock_guard<boost::recursive_mutex> lock(m_runningInstancesMutex);
         auto instance = getRunningInstance(id);

         YADOMS_LOG(debug) << "Send binding query " << request->getData().getQuery() << " to plugin " << instance->about()->DisplayName();

         instance->postBindingQueryRequest(request);
      }
      catch (CPluginException& e)
      {
         request->sendError((boost::format("Error when requesting binding query %1%") % e.what()).str());
      }
   }
} // namespace pluginSystem
