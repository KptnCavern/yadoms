#pragma once

#include "ITransceiver.h"
#include "rfxcomMessages/IRfxcomMessage.h"
#include "ISequenceNumberProvider.h"
#include "IUnsecuredProtocolFilter.h"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	This class implement the RFXCom protocol
//--------------------------------------------------------------
class CTransceiver : public ITransceiver
{
public:
   //--------------------------------------------------------------
   /// \brief	                           Constructor
   //--------------------------------------------------------------
   CTransceiver();

   //--------------------------------------------------------------
   /// \brief	                           Destructor
   //--------------------------------------------------------------
   virtual ~CTransceiver();

   // ITransceiver implementation
   shared::communication::CByteBuffer buildResetCmd() const override;
   shared::communication::CByteBuffer buildGetStatusCmd() const override;
   shared::communication::CByteBuffer buildSetModeCmd(unsigned char frequency,
                                                      const IRfxcomConfiguration& configuration) const override;
   shared::communication::CByteBuffer buildStartReceiverCmd() const override;
   boost::shared_ptr<std::queue<shared::communication::CByteBuffer> > buildMessageToDevice(boost::shared_ptr<yApi::IYPluginApi> api,
                                                                                           boost::shared_ptr<const yApi::IDeviceCommand> command) const override;
   boost::shared_ptr<rfxcomMessages::IRfxcomMessage> decodeRfxcomMessage(boost::shared_ptr<yApi::IYPluginApi> api,
                                                                         const shared::communication::CByteBuffer& data) const override;
   std::string createDeviceManually(boost::shared_ptr<yApi::IYPluginApi> api,
                                    const yApi::IManuallyDeviceCreationData& data) const override;
   // [END] ITransceiver implementation

private:

   static std::map<int, boost::shared_ptr<IUnsecuredProtocolFilter> > createUnsecuredProtocolFilters();

   //--------------------------------------------------------------
   /// \brief  The message sequence number
   //--------------------------------------------------------------
   boost::shared_ptr<ISequenceNumberProvider> m_seqNumberProvider;

   //--------------------------------------------------------------
   /// \brief  The list of filters used for unsecured protocols, to filter bad receptions and avoid bad device creations
   //--------------------------------------------------------------
   const std::map<int, boost::shared_ptr<IUnsecuredProtocolFilter> > m_unsecuredProtocolFilters;
};

