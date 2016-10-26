#include "stdafx.h"
#include "Instance.h"

namespace pluginSystem
{
   CInstance::CInstance(boost::shared_ptr<const database::entities::CPlugin> instanceInformation,
                        const boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                        const boost::filesystem::path& dataPath,
                        boost::shared_ptr<shared::process::IProcess> process,
                        boost::shared_ptr<IIpcAdapter> ipcAdapter)
      : m_instanceInformation(instanceInformation),
        m_pluginInformation(pluginInformation),
        m_process(process),
        m_ipcAdapter(ipcAdapter)
   {
      postInit(m_pluginInformation,
               dataPath);
   }

   CInstance::~CInstance()
   {
   }

   void CInstance::requestStop()
   {
      postStopRequest();
   }

   void CInstance::kill()
   {
      m_process->kill();
   }

   void CInstance::postStopRequest() const
   {
      m_ipcAdapter->postStopRequest();
   }

   void CInstance::postInit(boost::shared_ptr<const shared::plugin::information::IInformation> information,
                            const boost::filesystem::path& dataPath) const
   {
      m_ipcAdapter->postInit(information,
                             dataPath);
   }

   void CInstance::postDeviceCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> deviceCommand)
   {
      m_ipcAdapter->postDeviceCommand(deviceCommand);
   }

   void CInstance::postBindingQueryRequest(boost::shared_ptr<shared::plugin::yPluginApi::IBindingQueryRequest> request)
   {
      m_ipcAdapter->postBindingQueryRequest(request);
   }

   void CInstance::postDeviceConfigurationSchemaRequest(boost::shared_ptr<shared::plugin::yPluginApi::IDeviceConfigurationSchemaRequest> request)
   {
      m_ipcAdapter->postDeviceConfigurationSchemaRequest(request);
   }

   void CInstance::postManuallyDeviceCreationRequest(boost::shared_ptr<shared::plugin::yPluginApi::IManuallyDeviceCreationRequest> request)
   {
      m_ipcAdapter->postManuallyDeviceCreationRequest(request);
   }

   void CInstance::postExtraCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IExtraCommand> extraCommand)
   {
      m_ipcAdapter->postExtraCommand(extraCommand);
   }

   void CInstance::updateConfiguration(const shared::CDataContainer& newConfiguration)
   {
      m_ipcAdapter->postUpdateConfiguration(newConfiguration);
   }

   boost::shared_ptr<const database::entities::CPlugin> CInstance::about() const
   {
      return m_instanceInformation;
   }

   boost::shared_ptr<const shared::plugin::information::IInformation> CInstance::aboutPlugin() const
   {
      return m_pluginInformation;
   }
} // namespace pluginSystem
