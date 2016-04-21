#include "stdafx.h"
#include "ContextAccessor.h"
#include <shared/communication/MessageQueueRemover.hpp>
#include <shared/Log.h>
#include <shared/exception/InvalidParameter.hpp>

namespace pluginSystem
{

const size_t CContextAccessor::m_maxMessages(100);
const size_t CContextAccessor::m_maxMessageSize(10000);

   CContextAccessor::CContextAccessor(boost::shared_ptr<shared::plugin::yPluginApi::IYPluginApi> yPluginApi)
      : m_pluginApi(yPluginApi),
        m_id(createId()),
        m_sendMessageQueueId(m_id + ".toPlugin"),
        m_receiveMessageQueueId(m_id + ".toYadoms"),
        m_sendMessageQueueRemover(m_sendMessageQueueId),
        m_receiveMessageQueueRemover(m_receiveMessageQueueId),
        m_sendMessageQueue(boost::interprocess::create_only, m_sendMessageQueueId.c_str(), m_maxMessages, m_maxMessageSize),
        m_receiveMessageQueue(boost::interprocess::create_only, m_receiveMessageQueueId.c_str(), m_maxMessages, m_maxMessageSize),
        m_sendBuffer(boost::make_shared<unsigned char[]>(m_sendMessageQueue.get_max_msg_size())),
        m_messageQueueReceiveThread(boost::thread(&CContextAccessor::messageQueueReceiveThreaded, this))
{
}

CContextAccessor::~CContextAccessor()
{
   m_messageQueueReceiveThread.interrupt();
   m_messageQueueReceiveThread.join();
}

std::string CContextAccessor::id() const
{
   return m_id;
}

std::string CContextAccessor::createId()
{
   std::stringstream ss;
   ss << "yPlugin." << boost::uuids::random_generator()();
   return ss.str();
}

void CContextAccessor::messageQueueReceiveThreaded()
{
   // Verify that the version of the library that we linked against is
   // compatible with the version of the headers we compiled against.
   GOOGLE_PROTOBUF_VERIFY_VERSION;

   YADOMS_LOG(information) << "Message queue ID : " << m_id;

   try
   {
      auto message(boost::make_shared<unsigned char[]>(m_receiveMessageQueue.get_max_msg_size()));
      size_t messageSize;
      unsigned int messagePriority;
      while (true)
      {
         try
         {
            // boost::interprocess::message_queue::receive is not responding to boost thread interruption, so we need to do some
            // polling and call boost::this_thread::interruption_point to exit properly
            // Note that boost::interprocess::message_queue::timed_receive requires universal time to work (can not use shared::currentTime::Provider)
            auto messageWasReceived = m_receiveMessageQueue.timed_receive(message.get(), m_receiveMessageQueue.get_max_msg_size(), messageSize, messagePriority,
               boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time() + boost::posix_time::seconds(1)));
            boost::this_thread::interruption_point();

            if (messageWasReceived)
               processMessage(message, messageSize);
         }
         catch (std::exception& ex)
         {
            YADOMS_LOG(error) << "Error receiving/processing queue message : " << ex.what();
         }
      }
   }
   catch (boost::interprocess::interprocess_exception& ex)
   {
      YADOMS_LOG(error) << "Error creating/using message queues (" << m_id << ") in context accessor : " << ex.what();
   }
   catch (boost::thread_interrupted&)
   {
   }
   catch (std::exception& ex)
   {
      YADOMS_LOG(error) << ex.what();
   }
   catch (...)
   {
      YADOMS_LOG(error) << "Unknown error";
   }

   YADOMS_LOG(debug) << "Close message queues";

   // Delete all global objects allocated by libprotobuf.
   google::protobuf::ShutdownProtobufLibrary();
}

boost::shared_ptr<shared::plugin::yPluginApi::IYPluginApi> CContextAccessor::api() const
{
   return m_pluginApi;
}

void CContextAccessor::send(const toPlugin::msg& pbMsg)
{
   if (!pbMsg.IsInitialized())
      throw std::overflow_error("CContextAccessor::send : message is not fully initialized");

   if (pbMsg.ByteSize() > static_cast<int>(m_sendMessageQueue.get_max_msg_size()))
      throw std::overflow_error((boost::format("CContextAccessor::send : message is too big (%1% bytes)") % pbMsg.ByteSize()).str());

   if (!pbMsg.SerializeToArray(m_sendBuffer.get(), m_sendMessageQueue.get_max_msg_size()))
      throw std::overflow_error("CContextAccessor::send : fail to serialize message (too big ?)");

   m_sendMessageQueue.send(m_sendBuffer.get(), pbMsg.GetCachedSize(), 0);
}

void CContextAccessor::processMessage(boost::shared_ptr<const unsigned char[]> message, size_t messageSize) const
{
   if (messageSize < 1)
      throw shared::exception::CInvalidParameter("messageSize");

   // Unserialize message
   toYadoms::msg request;
   if (!request.ParseFromArray(message.get(), messageSize))
      throw shared::exception::CInvalidParameter("message");

   // Process message
   switch(request.OneOf_case())
   {
      case toYadoms::msg::kPluginState: processSetPluginState(request.pluginstate()); break;
      //TODO
   //case pbRequest::msg::kGetKeywordId: processGetKeywordId(request.getkeywordid(), messageQueue); break;
   //case pbRequest::msg::kGetRecipientId: processGetRecipientId(request.getrecipientid(), messageQueue); break;
   //case pbRequest::msg::kReadKeyword: processReadKeyword(request.readkeyword(), messageQueue); break;
   //case pbRequest::msg::kWaitForNextAcquisition: processWaitForNextAcquisition(request.waitfornextacquisition(), messageQueue); break;
   //case pbRequest::msg::kWaitForNextAcquisitions: processWaitForNextAcquisitions(request.waitfornextacquisitions(), messageQueue); break;
   //case pbRequest::msg::kWaitForEvent: processWaitForEvent(request.waitforevent(), messageQueue); break;
   //case pbRequest::msg::kWriteKeyword: processWriteKeyword(request.writekeyword(), messageQueue); break;
   //case pbRequest::msg::kSendNotification: processSendNotification(request.sendnotification(), messageQueue); break;
   //case pbRequest::msg::kGetInfo: processGetInfo(request.getinfo(), messageQueue); break;
   default:
      throw shared::exception::CInvalidParameter("message");
   }
}

void CContextAccessor::processSetPluginState(const toYadoms::SetPluginState& request) const
{
   shared::plugin::yPluginApi::historization::EPluginState state;
   switch (request.pluginstate())
   {
   case toYadoms::SetPluginState_EPluginState_kUnknown: state = shared::plugin::yPluginApi::historization::EPluginState::kUnknownValue; break;
   case toYadoms::SetPluginState_EPluginState_kError: state = shared::plugin::yPluginApi::historization::EPluginState::kErrorValue;  break;
   case toYadoms::SetPluginState_EPluginState_kStopped: state = shared::plugin::yPluginApi::historization::EPluginState::kStoppedValue;  break;
   case toYadoms::SetPluginState_EPluginState_kRunning: state = shared::plugin::yPluginApi::historization::EPluginState::kRunningValue;  break;
   case toYadoms::SetPluginState_EPluginState_kCustom: state = shared::plugin::yPluginApi::historization::EPluginState::kCustomValue;  break;
   default:
      throw std::out_of_range((boost::format("Unsupported plugin state received : %1%") % request.pluginstate()).str());
   }

   m_pluginApi->setPluginState(state, request.custommessageid());
}

//TODO
//void CContextAccessor::processGetKeywordId(const pbRequest::GetKeywordId& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::GetKeywordId* answer = ans.mutable_getkeywordid();
//   try
//   {
//      answer->set_id(m_scriptApi->getKeywordId(request.devicename(), request.keywordname()));
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processGetRecipientId(const pbRequest::GetRecipientId& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::GetRecipientId* answer = ans.mutable_getrecipientid();
//   try
//   {
//      answer->set_id(m_scriptApi->getRecipientId(request.firstname(), request.lastname()));
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processReadKeyword(const pbRequest::ReadKeyword& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::ReadKeyword* answer = ans.mutable_readkeyword();
//   try
//   {
//      answer->set_value(m_scriptApi->readKeyword(request.keywordid()));
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processWaitForNextAcquisition(const pbRequest::WaitForNextAcquisition& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::WaitForNextAcquisition* answer = ans.mutable_waitfornextacquisition();
//   try
//   {
//      answer->set_acquisition(m_scriptApi->waitForNextAcquisition(request.keywordid(), request.has_timeout() ? request.timeout() : std::string()));
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processWaitForNextAcquisitions(const pbRequest::WaitForNextAcquisitions& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::WaitForNextAcquisitions* answer = ans.mutable_waitfornextacquisitions();
//   try
//   {
//      std::vector<int> keywordIdList;
//      for (google::protobuf::RepeatedField<google::protobuf::int32>::const_iterator it = request.keywordid().begin(); it != request.keywordid().end(); ++it)
//         keywordIdList.push_back(*it);
//      std::pair<int, std::string> result = m_scriptApi->waitForNextAcquisitions(keywordIdList, request.has_timeout() ? request.timeout() : std::string());
//      answer->set_keywordid(result.first);
//      if (!result.second.empty())
//         answer->set_acquisition(result.second);
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processWaitForEvent(const pbRequest::WaitForEvent& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::WaitForEvent* answer = ans.mutable_waitforevent();
//   try
//   {
//      std::vector<int> keywordIdList;
//      for (google::protobuf::RepeatedField<google::protobuf::int32>::const_iterator it = request.keywordid().begin(); it != request.keywordid().end(); ++it)
//         keywordIdList.push_back(*it);
//      shared::script::yScriptApi::CWaitForEventResult result = m_scriptApi->waitForEvent(keywordIdList, request.receivedatetimeevent(), request.has_timeout() ? request.timeout() : std::string());
//      switch (result.getType())
//      {
//      case shared::script::yScriptApi::CWaitForEventResult::kTimeout:answer->set_type(pbAnswer::WaitForEvent_EventType_kTimeout); break;
//      case shared::script::yScriptApi::CWaitForEventResult::kKeyword:answer->set_type(pbAnswer::WaitForEvent_EventType_kKeyword); break;
//      case shared::script::yScriptApi::CWaitForEventResult::kDateTime:answer->set_type(pbAnswer::WaitForEvent_EventType_kDateTime); break;
//      default:
//         throw shared::exception::CInvalidParameter("CWaitForEventResult::type");
//      }
//      answer->set_keywordid(result.getKeywordId());
//      if (!result.getValue().empty())
//         answer->set_acquisition(result.getValue());
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processWriteKeyword(const pbRequest::WriteKeyword& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   ans.mutable_writekeyword();
//   try
//   {
//      m_scriptApi->writeKeyword(request.keywordid(), request.newstate());
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processSendNotification(const pbRequest::SendNotification& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   ans.mutable_sendnotification();
//   try
//   {
//      m_scriptApi->sendNotification(request.keywordid(), request.recipientid(), request.message());
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}
//
//void CContextAccessor::processGetInfo(const pbRequest::GetInfo& request, boost::interprocess::message_queue& messageQueue)
//{
//   pbAnswer::msg ans;
//   pbAnswer::GetInfo* answer = ans.mutable_getinfo();
//   try
//   {
//      shared::script::yScriptApi::IYScriptApi::EInfoKeys key;
//      switch (request.key())
//      {
//      case pbRequest::GetInfo_Key_kSunrise: key = shared::script::yScriptApi::IYScriptApi::kSunrise; break;
//      case pbRequest::GetInfo_Key_kSunset: key = shared::script::yScriptApi::IYScriptApi::kSunset; break;
//      case pbRequest::GetInfo_Key_kLatitude: key = shared::script::yScriptApi::IYScriptApi::kLatitude; break;
//      case pbRequest::GetInfo_Key_kLongitude: key = shared::script::yScriptApi::IYScriptApi::kLongitude; break;
//      case pbRequest::GetInfo_Key_kAltitude: key = shared::script::yScriptApi::IYScriptApi::kAltitude; break;
//      case pbRequest::GetInfo_Key_kYadomsServerOS: key = shared::script::yScriptApi::IYScriptApi::kYadomsServerOS; break;
//      case pbRequest::GetInfo_Key_kYadomsServerVersion: key = shared::script::yScriptApi::IYScriptApi::kYadomsServerVersion; break;
//      default:
//         throw shared::exception::CInvalidParameter("answer.waitforeventrequestanswer.type");
//      }
//
//      answer->set_value(m_scriptApi->getInfo(key));
//   }
//   catch (std::exception& ex)
//   {
//      ans.set_error(ex.what());
//   }
//   send(ans, messageQueue);
//}

} // namespace pluginSystem