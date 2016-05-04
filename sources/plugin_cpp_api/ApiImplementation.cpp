#include "stdafx.h"
#include "ApiImplementation.h"
#include <shared/DataContainer.h>
#include "PluginInformation.h"
#include "BindingQuery.h"
#include "DeviceCommand.h"
#include "ExtraCommand.h"
#include "ManuallyDeviceCreation.h"


namespace plugin_cpp_api
{
   CApiImplementation::CApiImplementation()
      : m_initialized(false),
        m_stopRequested(false)
   {
   }

   CApiImplementation::~CApiImplementation()
   {
   }

   void CApiImplementation::setSendingMessageQueue(boost::shared_ptr<boost::interprocess::message_queue> sendMessageQueue)
   {
      m_sendMessageQueue = sendMessageQueue;
      m_mqBuffer = boost::make_shared<unsigned char[]>(m_sendMessageQueue->get_max_msg_size());
   }

   bool CApiImplementation::stopRequested() const
   {
      return m_stopRequested;
   }

   void CApiImplementation::send(const toYadoms::msg& msg) const
   {
      try
      {
         if (!m_sendMessageQueue)
            throw std::runtime_error((boost::format("CApiImplementation::send \"%1%\", plugin API not ready to send message") % msg.descriptor()->full_name()).str());

         if (!msg.IsInitialized())
            throw std::runtime_error((boost::format("CApiImplementation::send \"%1%\", request is not fully initialized") % msg.descriptor()->full_name()).str());

         if (msg.ByteSize() > static_cast<int>(m_sendMessageQueue->get_max_msg_size()))
            throw std::runtime_error((boost::format("CApiImplementation::send \"%1%\", request is too big") % msg.descriptor()->full_name()).str());

         if (!msg.SerializeToArray(m_mqBuffer.get(), m_sendMessageQueue->get_max_msg_size()))
            throw std::runtime_error((boost::format("CApiImplementation::send \"%1%\", fail to serialize request (too big ?)") % msg.descriptor()->full_name()).str());

         m_sendMessageQueue->send(m_mqBuffer.get(), msg.GetCachedSize(), 0);
      }
      catch (boost::interprocess::interprocess_exception& ex)
      {
         throw std::runtime_error((boost::format("CApiImplementation::send \"%1%\" : error :%2%") % msg.descriptor()->full_name() % ex.what()).str());
      }
   }

   void CApiImplementation::send(const toYadoms::msg& msg,
                                 boost::function1<bool, const toPlugin::msg&> checkExpectedMessageFunction,
                                 boost::function1<void, const toPlugin::msg&> onReceiveFunction) const
   {
      shared::event::CEventHandler receivedEvtHandler;

      {
         boost::lock_guard<boost::recursive_mutex> lock(m_onReceiveHookMutex);
         m_onReceiveHook = [&](const toPlugin::msg& receivedMsg)-> bool
            {
               if (!checkExpectedMessageFunction(receivedMsg))
                  return false;

               receivedEvtHandler.postEvent<const toPlugin::msg>(shared::event::kUserFirstId, receivedMsg);
               return true;
            };
      }

      send(msg);

      if (receivedEvtHandler.waitForEvents(boost::posix_time::seconds(10)) == shared::event::kTimeout)
         throw std::runtime_error((boost::format("No answer from Yadoms when sending message %1%") % msg.OneOf_case()).str());

      onReceiveFunction(receivedEvtHandler.getEventData<const toPlugin::msg>());
   }

   void CApiImplementation::onReceive(boost::shared_ptr<const unsigned char[]> message,
                                      size_t messageSize)
   {
      if (messageSize < 1)
         throw std::runtime_error("CApiImplementation::onReceive : received Yadoms answer is zero length");

      toPlugin::msg toPluginProtoBuffer;
      if (!toPluginProtoBuffer.ParseFromArray(message.get(), messageSize))
         throw shared::exception::CInvalidParameter("message");

      if (!m_initialized)
      {
         if (toPluginProtoBuffer.has_plugininformation())
            processPluginInformation(toPluginProtoBuffer.plugininformation());
         else
            throw shared::exception::CInvalidParameter((boost::format("Unexpected message %1% when initialization") % toPluginProtoBuffer.OneOf_case()).str());
         return;
      }

      {
         boost::lock_guard<boost::recursive_mutex> lock(m_onReceiveHookMutex);
         if (m_onReceiveHook && m_onReceiveHook(toPluginProtoBuffer))
         {
            m_onReceiveHook.clear();
            return;
         }
      }

      switch (toPluginProtoBuffer.OneOf_case())
      {
      case toPlugin::msg::kSystem: processSystem(toPluginProtoBuffer.system());
         break;
      case toPlugin::msg::kPluginInformation: processPluginInformation(toPluginProtoBuffer.plugininformation());
         break;
      case toPlugin::msg::kConfiguration: processUpdateConfiguration(toPluginProtoBuffer.configuration());
         break;
      case toPlugin::msg::kBindingQuery: processBindingQuery(toPluginProtoBuffer.bindingquery());
         break;
      case toPlugin::msg::kDeviceCommand: processDeviceCommand(toPluginProtoBuffer.devicecommand());
         break;
      case toPlugin::msg::kExtraCommand: processExtraCommand(toPluginProtoBuffer.extracommand());
         break;
      case toPlugin::msg::kManuallyDeviceCreation: processManuallyDeviceCreation(toPluginProtoBuffer.manuallydevicecreation());
         break;
      default:
         throw shared::exception::CInvalidParameter("message");
      }
   }


   void CApiImplementation::waitInitialized() const
   {
      std::unique_lock<std::mutex> lock(m_initializationConditionMutex);
      if (!m_initialized)
         m_initializationCondition.wait(lock);
   }

   void CApiImplementation::processSystem(const toPlugin::System& msg)
   {
      if (msg.type() == toPlugin::System_EventType_kRequestStop)
      {
         // Request the main thread to stop
         getEventHandler().postEvent(kEventStopRequested);

         // Stop this thread
         throw boost::thread_interrupted();
      }
   }

   void CApiImplementation::processPluginInformation(const toPlugin::Information& msg)
   {
      m_pluginInformation = boost::make_shared<CPluginInformation>(boost::make_shared<const toPlugin::Information>(msg));
      setInitialized();
   }

   void CApiImplementation::processUpdateConfiguration(const toPlugin::Configuration& msg)
   {
      m_pluginEventHandler.postEvent(kEventUpdateConfiguration, shared::CDataContainer(msg.configuration()));
   }

   void CApiImplementation::setInitialized()
   {
      if (!!m_pluginInformation)
      {
         std::unique_lock<std::mutex> lock(m_initializationConditionMutex);
         m_initialized = true;
         m_initializationCondition.notify_one();
      }
   }

   void CApiImplementation::processBindingQuery(const toPlugin::BindingQuery& msg)
   {
      boost::shared_ptr<shared::plugin::yPluginApi::IBindingQueryRequest> query = boost::make_shared<CBindingQuery>(msg,
                                                                                                                    [&](const shared::CDataContainer& r)
                                                                                                                    {
                                                                                                                       toYadoms::msg ans;
                                                                                                                       auto answer = ans.mutable_bindingqueryanswer();
                                                                                                                       answer->set_success(true);
                                                                                                                       answer->set_result(r.serialize());
                                                                                                                       send(ans);
                                                                                                                    },
                                                                                                                    [&](const std::string& r)
                                                                                                                    {
                                                                                                                       toYadoms::msg ans;
                                                                                                                       auto answer = ans.mutable_bindingqueryanswer();
                                                                                                                       answer->set_success(false);
                                                                                                                       answer->set_result(r);
                                                                                                                       send(ans);
                                                                                                                    });

      m_pluginEventHandler.postEvent(kBindingQuery, query);
   }

   void CApiImplementation::processDeviceCommand(const toPlugin::DeviceCommand& msg)
   {
      m_pluginEventHandler.postEvent(kEventDeviceCommand, boost::make_shared<CDeviceCommand>(msg));
   }

   void CApiImplementation::processExtraCommand(const toPlugin::ExtraCommand& msg)
   {
      m_pluginEventHandler.postEvent(kEventExtraCommand, boost::make_shared<CExtraCommand>(msg));
   }

   void CApiImplementation::processManuallyDeviceCreation(const toPlugin::ManuallyDeviceCreation& msg)
   {
      boost::shared_ptr<shared::plugin::yPluginApi::IManuallyDeviceCreationRequest> request =
         boost::make_shared<CManuallyDeviceCreation>(msg,
                                                     [&](const std::string& newDeviceName)
                                                     {
                                                        toYadoms::msg ans;
                                                        auto answer = ans.mutable_manuallydevicecreationanswer();
                                                        auto success = answer->mutable_sucess();
                                                        success->set_newdevicename(newDeviceName);
                                                        send(ans);
                                                     },
                                                     [&](const std::string& errorMessage)
                                                     {
                                                        toYadoms::msg ans;
                                                        auto answer = ans.mutable_manuallydevicecreationanswer();
                                                        auto error = answer->mutable_error();
                                                        error->set_message(errorMessage);
                                                        send(ans);
                                                     });

      m_pluginEventHandler.postEvent(kEventManuallyDeviceCreation, request);
   }

   void CApiImplementation::setPluginState(const shared::plugin::yPluginApi::historization::EPluginState& state,
                                           const std::string& customMessageId)
   {
      toYadoms::msg req;
      auto request = req.mutable_pluginstate();
      switch (state)
      {
      case shared::plugin::yPluginApi::historization::EPluginState::kUnknownValue: request->set_pluginstate(toYadoms::SetPluginState_EPluginState_kUnknown);
         break;
      case shared::plugin::yPluginApi::historization::EPluginState::kErrorValue: request->set_pluginstate(toYadoms::SetPluginState_EPluginState_kError);
         break;
      case shared::plugin::yPluginApi::historization::EPluginState::kStoppedValue: request->set_pluginstate(toYadoms::SetPluginState_EPluginState_kStopped);
         break;
      case shared::plugin::yPluginApi::historization::EPluginState::kRunningValue: request->set_pluginstate(toYadoms::SetPluginState_EPluginState_kRunning);
         break;
      case shared::plugin::yPluginApi::historization::EPluginState::kCustomValue: request->set_pluginstate(toYadoms::SetPluginState_EPluginState_kCustom);
         break;
      default:
         throw std::out_of_range((boost::format("CApiImplementation::setPluginState, unknown state %1%") % state).str());
      }
      request->set_custommessageid(customMessageId);
      send(req);
   }

   bool CApiImplementation::deviceExists(const std::string& device) const
   {
      toYadoms::msg req;
      auto request = req.mutable_deviceexists();
      request->set_device(device);

      auto exists = false;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_deviceexists();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              exists = ans.deviceexists().exists();
           });

      return exists;
   }

   shared::CDataContainer CApiImplementation::getDeviceDetails(const std::string& device) const
   {
      toYadoms::msg req;
      auto request = req.mutable_devicedetails();
      request->set_device(device);

      shared::CDataContainer details;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_devicedetails();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              details.deserialize(ans.devicedetails().details());
           });

      return details;
   }

   void CApiImplementation::declareDevice(const std::string& device,
                                          const std::string& model, const shared::CDataContainer& details)
   {
      toYadoms::msg req;
      auto request = req.mutable_declaredevice();
      request->set_device(device);
      request->set_model(model);
      if (!details.empty())
         request->set_details(details.serialize());
      send(req);
   }

   bool CApiImplementation::keywordExists(const std::string& device,
                                          const std::string& keyword) const
   {
      toYadoms::msg req;
      auto request = req.mutable_keywordexists();
      request->set_device(device);
      request->set_keyword(keyword);

      auto exists = false;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_keywordexists();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              exists = ans.keywordexists().exists();
           });

      return exists;
   }

   bool CApiImplementation::keywordExists(const std::string& device,
                                          const shared::plugin::yPluginApi::historization::IHistorizable& keyword) const
   {
      return keywordExists(device, keyword.getKeyword());
   }

   void CApiImplementation::declareKeyword(const std::string& device,
                                           const shared::plugin::yPluginApi::historization::IHistorizable& keyword,
                                           const shared::CDataContainer& details)
   {
      toYadoms::msg req;
      auto request = req.mutable_declarekeyword();
      request->set_device(device);
      fillHistorizable(keyword, request->mutable_keyword());
      if (!details.empty())
         request->set_details(details.serialize());
      send(req);
   }

   void CApiImplementation::fillHistorizable(const shared::plugin::yPluginApi::historization::IHistorizable& in,
                                             toYadoms::Historizable* out)
   {
      fillCapacity(in.getCapacity(), out->mutable_capacity());
      out->set_accessmode(in.getAccessMode().toString());
      out->set_name(in.getKeyword());
      out->set_type(in.getCapacity().getType().toString());
      out->set_units(in.getCapacity().getUnit());
      out->set_typeinfo(in.getTypeInfo().serialize());
      out->set_measure(in.getMeasureType().toString());
   }

   void CApiImplementation::fillCapacity(const shared::plugin::yPluginApi::CStandardCapacity& in,
                                         toYadoms::Capacity* out)
   {
      out->set_name(in.getName());
      out->set_unit(in.getUnit());
      out->set_type(in.getType().toString());
   }

   std::string CApiImplementation::getRecipientValue(int recipientId,
                                                     const std::string& fieldName) const
   {
      toYadoms::msg req;
      auto request = req.mutable_recipientvaluerequest();
      request->set_recipientid(recipientId);
      request->set_fieldname(fieldName);

      std::string value;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_recipientvalue();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              value = ans.recipientvalue().value();
           });

      return value;
   }

   std::vector<int> CApiImplementation::findRecipientsFromField(const std::string& fieldName,
                                                                const std::string& expectedFieldValue) const
   {
      toYadoms::msg req;
      auto request = req.mutable_findrecipientsfromfieldrequest();
      request->set_fieldname(fieldName);
      request->set_expectedfieldvalue(expectedFieldValue);

      std::vector<int> recipientIds;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_findrecipientsfromfieldanswer();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              std::copy(ans.findrecipientsfromfieldanswer().recipientids().begin(), ans.findrecipientsfromfieldanswer().recipientids().end(), std::back_inserter(recipientIds));
           });

      return recipientIds;
   }

   bool CApiImplementation::recipientFieldExists(const std::string& fieldName) const
   {
      toYadoms::msg req;
      auto request = req.mutable_recipientfieldexitsrequest();
      request->set_fieldname(fieldName);

      auto exists = false;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_keywordexists();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              exists = ans.keywordexists().exists();
           });

      return exists;
   }

   void CApiImplementation::historizeData(const std::string& device,
                                          const shared::plugin::yPluginApi::historization::IHistorizable& data)
   {
      toYadoms::msg msg;
      auto message = msg.mutable_historizedata();
      message->set_device(device);
      auto value = message->add_value();
      fillHistorizable(data, value->mutable_historizable());
      value->set_formattedvalue(data.formatValue());
      send(msg);
   }

   void CApiImplementation::historizeData(const std::string& device,
                                          const std::vector<boost::shared_ptr<shared::plugin::yPluginApi::historization::IHistorizable>>& dataVect)
   {
      toYadoms::msg msg;
      auto message = msg.mutable_historizedata();
      message->set_device(device);
      for (auto data = dataVect.begin(); data != dataVect.end(); ++data)
      {
         auto value = message->add_value();
         fillHistorizable(**data, value->mutable_historizable());
         value->set_formattedvalue((*data)->formatValue());
      }
      send(msg);
   }

   boost::shared_ptr<const shared::plugin::information::IInformation> CApiImplementation::getInformation() const
   {
      if (!m_pluginInformation)
         throw std::exception("Plugin information not available");

      return m_pluginInformation;
   }

   shared::CDataContainer CApiImplementation::getConfiguration()
   {
      toYadoms::msg req;
      req.mutable_configurationrequest();

      shared::CDataContainer configuration;
      send(req,
           [](const toPlugin::msg& ans) -> bool
           {
              return ans.has_configuration();
           },
           [&](const toPlugin::msg& ans) -> void
           {
              configuration.deserialize(ans.configuration().configuration());
           });

      return configuration;
   }

   shared::event::CEventHandler& CApiImplementation::getEventHandler()
   {
      return m_pluginEventHandler;
   }
} // namespace plugin_cpp_api	


