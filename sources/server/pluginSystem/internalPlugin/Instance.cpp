#include "stdafx.h"
#include "Instance.h"

namespace pluginSystem
{
   namespace internalPlugin
   {
      CInstance::CInstance(boost::shared_ptr<const database::entities::CPlugin> instanceInformation,
                           boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                           boost::shared_ptr<shared::plugin::yPluginApi::IYPluginApi> api,
                           boost::shared_ptr<CInstanceStateHandler> instanceStateHandler)
         : m_instanceInformation(instanceInformation),
           m_pluginInformation(pluginInformation),
           m_apiEventHandler(api->getEventHandler()),
           m_thread(&CInstance::Thread, api, instanceStateHandler)
      {
      }

      CInstance::~CInstance()
      {
      }

      void CInstance::requestStop()
      {
         m_thread.interrupt();
      }

      void CInstance::kill()
      {
         m_thread.interrupt();
         m_thread.join();
      }

      void CInstance::Thread(boost::shared_ptr<shared::plugin::yPluginApi::IYPluginApi> api,
                             boost::shared_ptr<CInstanceStateHandler> instanceStateHandler)
      {
         instanceStateHandler->onStart();

         CSystem system;
         system.doWork(api);

         instanceStateHandler->onFinish(0, std::string());
      }

      void CInstance::postDeviceCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> deviceCommand) const
      {
         m_apiEventHandler.postEvent(yApi::IYPluginApi::kEventDeviceCommand, deviceCommand);
      }

      void CInstance::postBindingQueryRequest(boost::shared_ptr<shared::plugin::yPluginApi::IBindingQueryRequest> request) const
      {
         m_apiEventHandler.postEvent(yApi::IYPluginApi::kBindingQuery, request);
      }

      void CInstance::postManuallyDeviceCreationRequest(boost::shared_ptr<shared::plugin::yPluginApi::IManuallyDeviceCreationRequest> request) const
      {
         m_apiEventHandler.postEvent(yApi::IYPluginApi::kEventManuallyDeviceCreation, request);
      }

      void CInstance::postExtraCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IExtraCommand> extraCommand) const
      {
         m_apiEventHandler.postEvent(yApi::IYPluginApi::kEventExtraCommand, extraCommand);
      }

      void CInstance::updateConfiguration(const shared::CDataContainer& newConfiguration) const
      {
         m_apiEventHandler.postEvent(yApi::IYPluginApi::kEventUpdateConfiguration, newConfiguration);
      }

      boost::shared_ptr<const database::entities::CPlugin> CInstance::about() const
      {
         return m_instanceInformation;
      }

      boost::shared_ptr<const shared::plugin::information::IInformation> CInstance::aboutPlugin() const
      {
         return m_pluginInformation;
      }
   }
} // namespace pluginSystem::internalPlugin


