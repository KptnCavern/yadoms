//
// Instance.h
//
// yadoms-plugin instance handler
//
#pragma once
#include <shared/plugin/information/IInformation.h>
#include <shared/plugin/yPluginApi/IBindingQueryRequest.h>
#include <shared/plugin/yPluginApi/IDeviceCommand.h>
#include <shared/plugin/yPluginApi/IExtraCommand.h>
#include "../InstanceStateHandler.h"
#include "../IInstance.h"
#include "System.h"

namespace pluginSystem
{
   namespace internalPlugin
   {
      //--------------------------------------------------------------
      /// \brief	this class is used to manage a plugin instance. 
      //--------------------------------------------------------------
      class CInstance : public IInstance
      {
      public:
         //--------------------------------------------------------------
         /// \brief	                           Constructor
         /// \param[in] instanceInformation     Instance information
         /// \param[in] pluginInformation       Plugin information
         /// \param[in] yPluginApi              yPluginApi context instance
         /// \param[in] instanceStateHandler    Instance state handler
         //--------------------------------------------------------------
         CInstance(boost::shared_ptr<const database::entities::CPlugin> instanceInformation,
                   boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                   boost::shared_ptr<shared::plugin::yPluginApi::IYPluginApi> api,
                   boost::shared_ptr<CInstanceStateHandler> instanceStateHandler);

         //--------------------------------------------------------------
         /// \brief	Destructor
         //--------------------------------------------------------------
         virtual ~CInstance();

         // IInstance Implementation
         void updateConfiguration(const shared::CDataContainer& newConfiguration) override;
         void requestStop() override;
         void kill() override;
         boost::shared_ptr<const database::entities::CPlugin> about() const override;
         boost::shared_ptr<const shared::plugin::information::IInformation> aboutPlugin() const override;
         void postDeviceCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> deviceCommand) override;
         void postExtraCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IExtraCommand> extraCommand) override;
         void postBindingQueryRequest(boost::shared_ptr<shared::plugin::yPluginApi::IBindingQueryRequest> request) override;
         void postManuallyDeviceCreationRequest(boost::shared_ptr<shared::plugin::yPluginApi::IManuallyDeviceCreationRequest> request) override;
         // [END] IInstance Implementation

      private:
         boost::shared_ptr<const database::entities::CPlugin> m_instanceInformation;
         boost::shared_ptr<const shared::plugin::information::IInformation> m_pluginInformation;
         boost::shared_ptr<shared::event::CEventHandler> m_eventHandler;
         boost::thread m_thread;
         static void Thread(boost::shared_ptr<shared::plugin::yPluginApi::IYPluginApi> api,
                            boost::shared_ptr<shared::event::CEventHandler> eventHandler,
                            boost::shared_ptr<CInstanceStateHandler> instanceStateHandler);
      };
   }
} // namespace pluginSystem::internalPlugin


