#pragma once

namespace shared
{
   namespace communication
   {
      //--------------------------------------------------------------
      /// \class An automaticaly message queue remover
      /// \details As a message queue is persistent on the system,
      /// we delete it :
      /// - on construction (for safety)
      /// - on destruction (for cleanup)
      //--------------------------------------------------------------
      struct CMessageQueueRemover
      {
         //--------------------------------------------------------------
         /// \brief	Constructor
         /// \param[in] messageQueueId The message queue ID (unique system-wide)
         //--------------------------------------------------------------
         explicit CMessageQueueRemover(const std::string& messageQueueId)
            : m_messageQueueId(messageQueueId)
         {
            boost::interprocess::message_queue::remove(m_messageQueueId.c_str());
         }

         //--------------------------------------------------------------
         /// \brief	Destructor
         //--------------------------------------------------------------
         virtual ~CMessageQueueRemover()
         {
            boost::interprocess::message_queue::remove(m_messageQueueId.c_str());
         }

      private:
         //--------------------------------------------------------------
         /// \brief	The message queue ID (unique system-wide)
         //--------------------------------------------------------------
         const std::string& m_messageQueueId;
      };
   }
} // namespace shared::communication


