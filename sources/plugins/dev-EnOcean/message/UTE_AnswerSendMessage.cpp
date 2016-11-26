#include "stdafx.h"
#include "UTE_AnswerSendMessage.h"
#include "../profiles/bitsetHelpers.hpp"

namespace message
{
   CUTE_AnswerSendMessage::CUTE_AnswerSendMessage(unsigned char status,
                                                  const std::string& destinationId,
                                                  bool bidirectionnalCommunication,
                                                  EResponse response,
                                                  unsigned char channelNumber,
                                                  unsigned int manufacturerId,
                                                  unsigned char type,
                                                  unsigned char func,
                                                  unsigned char rorg)
      : CRadioErp1SendMessage(CRorgs::kUTE_Telegram,
                              "00000000",//TODO mettre constant
                              status),
        m_destinationId(destinationId),
        m_bidirectionalCommunication(bidirectionnalCommunication),
        m_response(response),
        m_command(kTeachInResponse),
        m_channelNumber(channelNumber),
        m_manufacturerId(manufacturerId),
        m_type(type),
        m_func(func),
        m_rorg(rorg)
   {
   }

   CUTE_AnswerSendMessage::~CUTE_AnswerSendMessage()
   {
   }

   boost::shared_ptr<const std::vector<unsigned char>> CUTE_AnswerSendMessage::buffer()
   {
      boost::dynamic_bitset<> db6(8);
      bitset_insert(db6, 0, 1, m_bidirectionalCommunication ? 1 : 0);
      bitset_insert(db6, 2, 2, m_response);
      bitset_insert(db6, 4, 4, m_command);

      std::vector<unsigned char> userData(7);
      userData[0] = bitset_to_byte(db6);
      userData[1] = m_channelNumber;
      userData[2] = m_manufacturerId & 0xFF;
      userData[3] = (m_manufacturerId >> 8) & 0x07;
      userData[4] = m_type;
      userData[5] = m_func;
      userData[6] = m_rorg;

      CRadioErp1SendMessage::userData(userData);


      enum
         {
            kSubTelNumSend = 3
         };
      enum
         {
            kDBmSubCase = 0xFF
         };
      enum
         {
            kTelegramUnencrypted = 0
         };

      std::vector<unsigned char> optionalData(7);
      optionalData[0] = kSubTelNumSend;
      optionalData[1] = static_cast<unsigned char>(std::stoul(m_destinationId.substr(0, 2), nullptr, 16));
      optionalData[2] = static_cast<unsigned char>(std::stoul(m_destinationId.substr(2, 2), nullptr, 16));
      optionalData[3] = static_cast<unsigned char>(std::stoul(m_destinationId.substr(4, 2), nullptr, 16));
      optionalData[4] = static_cast<unsigned char>(std::stoul(m_destinationId.substr(6, 2), nullptr, 16));
      optionalData[5] = kDBmSubCase;
      optionalData[6] = kTelegramUnencrypted;

      CRadioErp1SendMessage::optionalData(optionalData);

      return CRadioErp1SendMessage::buffer();
   }
} // namespace message
