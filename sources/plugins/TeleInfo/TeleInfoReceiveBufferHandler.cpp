#include "stdafx.h"
#include "TeleInfoReceiveBufferHandler.h"

enum
{
   kSTX = 0x02,
   kETX = 0x03,

   kStartMessage = 0x0a,
   kEndMessage = 0x0d
};

enum
{
   maxMessageSize = 1000
};

CTeleInfoReceiveBufferHandler::CTeleInfoReceiveBufferHandler(shared::event::CEventHandler& receiveDataEventHandler,
                                                             int receiveDataEventId,
                                                             const boost::posix_time::time_duration suspendDelay,
															 boost::shared_ptr<shared::communication::IBufferLogger> logger)
   : m_receiveDataEventHandler(receiveDataEventHandler),
     m_receiveDataEventId(receiveDataEventId),
     m_nextSendMessageDate(shared::currentTime::Provider().now()),
	 m_logger (logger),
     m_suspendDelay (suspendDelay)
{
}

CTeleInfoReceiveBufferHandler::~CTeleInfoReceiveBufferHandler()
{
}

void CTeleInfoReceiveBufferHandler::push(const shared::communication::CByteBuffer& buffer)
{
   if (shared::currentTime::Provider().now() < m_nextSendMessageDate)
      return;

   for (size_t idx = 0; idx < buffer.size(); ++idx)
	   m_content.push_back(buffer[idx]);

   // Send message if complete (separate aggregated messages)
   while (true)
   {
      m_logger->logReceived(shared::communication::CByteBuffer(m_content));

      const auto messages = getCompleteMessage();
      if (messages->empty())
         break;
      notifyEventHandler(messages);

      m_nextSendMessageDate = shared::currentTime::Provider().now() + m_suspendDelay;
   }
}

void CTeleInfoReceiveBufferHandler::flush()
{
   m_content.clear();
}

boost::shared_ptr<std::map<std::string, std::string>> CTeleInfoReceiveBufferHandler::getCompleteMessage()
{
   static const auto noMessages = boost::make_shared<std::map<std::string, std::string>>();

   if (m_content.empty())
      return noMessages;

   // Remove first bytes if not sync byte
   while (!m_content.empty() && m_content[0] != kSTX)
      m_content.erase(m_content.begin());

   if (m_content.empty())
      return noMessages;

   if (m_content[1] != kStartMessage)
	   return noMessages;

   auto etxIterator = std::find(m_content.rbegin(), m_content.rend(), kETX);
   if (etxIterator == m_content.rend())
   {
      if (m_content.size() > maxMessageSize)
         m_content.clear();

      return noMessages;
   }

   size_t etxPosition = std::distance(std::begin(m_content), etxIterator.base()) - 1;

   // The message is complete
   auto frame = boost::make_shared<std::vector<unsigned char>>(m_content.begin(), m_content.begin() + etxPosition + 1);
   m_content.erase(m_content.begin(), m_content.begin() + etxPosition);

   const auto messages = getMessages(frame);
   if (messages->empty())
      return noMessages;

   return messages;
}

boost::shared_ptr<std::map<std::string, std::string>> CTeleInfoReceiveBufferHandler::getMessages(boost::shared_ptr<const std::vector<unsigned char>> frame)
{
   static const auto noMessages = boost::make_shared<std::map<std::string, std::string>>();
   auto messages = boost::make_shared<std::map<std::string, std::string>>();
   auto endPos = frame->begin();
   while (true)
   {
      auto startPos = std::find(endPos, frame->end(), kStartMessage);
      if (startPos == frame->end())
         return noMessages;

      endPos = std::find(startPos, frame->end(), kEndMessage);
	  if (endPos == frame->end())
		  return noMessages;

	  ++endPos;
      auto message = std::string(startPos, endPos);
      if (!isCheckSumOk(message))
         return noMessages;

      // Remove <cr> and <lf>
	  message.pop_back();
      message.erase(message.begin());

      // Separate key/value
      boost::char_separator<char> sep(" ");
      boost::tokenizer<boost::char_separator<char>> tok(message, sep);

      try
      {
         auto iterator = tok.begin();

		 if (iterator != tok.end())
		 {
			 const auto key = *iterator;
			 ++iterator;

			 if (iterator != tok.end())
			 {
				 const auto value = *iterator;
				 (*messages)[key] = value;
			 }
			 else
				return noMessages;
		 }
		 else
			return noMessages;
      }
      catch (...)
      {
         return noMessages;
      }

      if (endPos + 1 == frame->end())
         return messages;
   }
}


/* Explanation of the checksum computation issued from the official EDF specification

A "checksum" is calculated on the set of characters from the beginning of the label field to the end of the field given character SP included.
We first make ??the sum of all ASCII codes of all characters.
to avoid introduce ASCII (00 to 1F hex) functions, it retains only the six least significant bits of
result (this translates into a logical AND between the amount previously calculated and 03Fh).
Finally, we added 20 hexadecimal. The result will always be a printable ASCII character (sign, digit,
capital letter) of from 0x20 to hexadecimal 0x5F

La "checksum" est calcul� sur l'ensemble des caract�res allant du d�but du champ �tiquette � la fin du champ
donn�e, caract�re SP inclus. On fait tout d'abord la somme des codes ASCII de tous ces caract�res. Pour �viter
d'introduire des fonctions ASCII (00 � 1F en hexad�cimal), on ne conserve que les six bits de poids faible du
r�sultat obtenu (cette op�ration se traduit par un ET logique entre la somme pr�c�demment calcul�e et 03Fh).
Enfin, on ajoute 20 en hexad�cimal. Le r�sultat sera donc toujours un caract�re ASCII imprimable (signe, chiffre,
lettre majuscule) allant de 20 � 5F en hexad�cimal.
*/
bool CTeleInfoReceiveBufferHandler::isCheckSumOk(const std::string& message)
{
   if (message.size() < 4)
      return false;

   auto checksum = 0x00;
   for (auto byte = message.begin() + 1; byte != (message.end() - 3); ++byte)
      checksum += *byte;

   checksum = (checksum & 0x3F) + 0x20;

   return checksum == message[message.size() - 2];
}

void CTeleInfoReceiveBufferHandler::notifyEventHandler(boost::shared_ptr<std::map<std::string, std::string>> messages) const
{
   m_receiveDataEventHandler.postEvent<boost::shared_ptr<std::map<std::string, std::string>>>(m_receiveDataEventId,
                                                                                              messages);
}
