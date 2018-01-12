#include "stdafx.h"
#include "RfxComReceiveBufferHandler.h"


CRfxcomReceiveBufferHandler::CRfxcomReceiveBufferHandler(shared::event::CEventHandler& receiveDataEventHandler,
                                                         const int receiveDataEventId)
   : m_lastReceivedTime(shared::currentTime::Provider().now()),
     m_receiveDataEventHandler(receiveDataEventHandler),
     m_receiveDataEventId(receiveDataEventId)
{
}

CRfxcomReceiveBufferHandler::~CRfxcomReceiveBufferHandler()
{
}

void CRfxcomReceiveBufferHandler::push(const shared::communication::CByteBuffer& buffer)
{
   static const auto InterByteTimeout = boost::posix_time::milliseconds(100);

   // Manage timeout
   const auto now = shared::currentTime::Provider().now();
   boost::lock_guard<boost::recursive_mutex> lock(m_contentMutex);
   if (m_content.size() != 0)
   {
      // Reset data if too old
      if (now - m_lastReceivedTime > InterByteTimeout)
         m_content.clear();
   }
   m_lastReceivedTime = now;

   for (unsigned int idx = 0; idx < buffer.size(); ++idx)
      m_content.push_back(buffer[idx]);

   // Send message if complete (separate aggregated messages)
   while (true)
   {
      const auto completeMessage = getCompleteMessage();
      if (!completeMessage)
         break;
      notifyEventHandler(completeMessage);
   }
}

void CRfxcomReceiveBufferHandler::flush()
{
   boost::lock_guard<boost::recursive_mutex> lock(m_contentMutex);
   m_content.clear();
}

boost::shared_ptr<const shared::communication::CByteBuffer> CRfxcomReceiveBufferHandler::getCompleteMessage()
{
   static const boost::shared_ptr<const shared::communication::CByteBuffer> UncompleteMessage;
   
   boost::lock_guard<boost::recursive_mutex> lock(m_contentMutex);
   if (m_content.empty())
      return UncompleteMessage;

   // The message size is provided in the first byte of the message
   // (see RFXCom specifications). This value counts all bytes except itself.
   // So a message is considered complete if its size is at least the value indicated
   // in the first byte + 1.
   const size_t extractedMessageSize = m_content[0] + 1;
   if (m_content.size() < extractedMessageSize)
      return UncompleteMessage;

   // The message is complete

   auto message = boost::make_shared<const shared::communication::CByteBuffer>(std::vector<unsigned char>(m_content.begin(),
                                                                                                          m_content.begin() + extractedMessageSize));

   // Delete extracted data
   m_content.erase(m_content.begin(), m_content.begin() + extractedMessageSize);

   return message;
}

void CRfxcomReceiveBufferHandler::notifyEventHandler(const boost::shared_ptr<const shared::communication::CByteBuffer> buffer) const
{
   m_receiveDataEventHandler.postEvent<const shared::communication::CByteBuffer>(m_receiveDataEventId, *buffer);
}

