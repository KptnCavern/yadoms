#pragma once
#include "types.h"

namespace message
{
   class CHeader
   {
   public:
      explicit CHeader(const std::vector<unsigned char>& receivedBuffer);

      unsigned short dataLength() const;
      unsigned char optionalLength() const;
      EPacketType packetType() const;
      int offsetData() const;
      int offsetOptional() const;

   private:
      const unsigned short m_dataLength;
      const unsigned char m_optionalLength;
      const EPacketType m_packetType;
      const int m_offsetData;
      const int m_offsetOptional;
   };


   inline unsigned short toWord(const std::vector<unsigned char>& content,
      const unsigned int offset)
   {
      return content[offset] + content[offset + 1];
   }


   //--------------------------------------------------------------
   /// \brief	EnOcean ESP3 received message
   ///
   /// This class manages an ESP3 EnOcean message, when receiving.
   //--------------------------------------------------------------
   class CReceivedEsp3Packet
   {
   public:
      //--------------------------------------------------------------
      /// \brief	                           Constructor
      /// \param[in] receivedContent         The received buffer
      //--------------------------------------------------------------
      explicit CReceivedEsp3Packet(const std::vector<unsigned char>& receivedBuffer);

      //--------------------------------------------------------------
      /// \brief	                           Destructor
      //--------------------------------------------------------------
      virtual ~CReceivedEsp3Packet();

      const CHeader& header() const;
      const std::vector<unsigned char>& data() const;
      const std::vector<unsigned char>& optional() const;

   private:
      const CHeader m_header;
      const std::vector<unsigned char> m_data;
      const std::vector<unsigned char> m_optional;
   };


   //--------------------------------------------------------------
   /// \brief	EnOcean RADIO_ERP1 message
   ///
   /// This class manages a RADIO_ERP1 EnOcean message.
   //--------------------------------------------------------------
   class CRadioErp1Message
   {
   public:
      enum ERORG
      {
         kRBS = 0xF6,
         k1BS = 0xD5,
         k4BS = 0xA5,
         kVLD = 0xD2,
         kMSC = 0xD1,
         kADT = 0xA6,
         kSM_LRN_REQ = 0xC6,
         kSM_LRN_ANS = 0xC7,
         kSM_REC = 0xA7,
         kSYS_EX = 0xC5,
         kSEC = 0x30,
         kSEC_ENCAPS = 0x31
      };

      //--------------------------------------------------------------
      /// \brief	                           Constructor
      /// \param[in] receivedContent         The received buffer
      //--------------------------------------------------------------
      explicit CRadioErp1Message(const CReceivedEsp3Packet& esp3Packet);

      //--------------------------------------------------------------
      /// \brief	                           Destructor
      //--------------------------------------------------------------
      virtual ~CRadioErp1Message();

      unsigned char rorg() const;
      unsigned int senderId() const;
      const std::vector<unsigned char>& data() const;

   private:
      const unsigned char m_rorg;
      const unsigned int m_senderId;
      const unsigned char m_status;
      const std::vector<unsigned char> m_data;
   };


   //--------------------------------------------------------------
   /// \brief	EnOcean 4BS radio message
   ///
   /// This class manages a 4BS radio EnOcean message.
   //--------------------------------------------------------------
   class C4BSMessage
   {
   public:
      //--------------------------------------------------------------
      /// \brief	                           Constructor
      /// \param[in] receivedContent         The received ERP1 radio message
      //--------------------------------------------------------------
      explicit C4BSMessage(const CRadioErp1Message& erp1);

      //--------------------------------------------------------------
      /// \brief	                           Destructor
      //--------------------------------------------------------------
      virtual ~C4BSMessage();

      unsigned char db3() const;
      unsigned char db2() const;
      unsigned char db1() const;
      unsigned char db0() const;

   private:
      const unsigned char m_db3;
      const unsigned char m_db2;
      const unsigned char m_db1;
      const unsigned char m_db0;
   };


} // namespace message
