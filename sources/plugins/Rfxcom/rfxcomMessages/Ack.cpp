#include "stdafx.h"
#include <shared/Log.h>
#include "Ack.h"


namespace rfxcomMessages
{

// Message size
static const std::size_t RXRESPONSE_size = sizeof(dummyRbufToComputeSizes.RXRESPONSE);

CAck::CAck(const RBUF& rbuf, boost::shared_ptr<const ISequenceNumberProvider> seqNumberProvider)
{
   CheckReceivedMessage(rbuf, pTypeRecXmitMessage, sTypeTransmitterResponse, RXRESPONSE_size, seqNumberProvider->last());

   m_ack = rbuf.RXRESPONSE.msg == 0x00;  // Ack is OK if rbuf->RXRESPONSE.msg == 0
}                         

CAck::~CAck()
{
}

const CByteBuffer CAck::encode(boost::shared_ptr<ISequenceNumberProvider> seqNumberProvider) const
{
   BOOST_ASSERT_MSG(false, "Ack is a read-only message, should not be encoded");
   return CByteBuffer();
}

void CAck::historizeData(boost::shared_ptr<yApi::IYadomsApi> context) const
{
   // Nothing to historize
}

} // namespace rfxcomMessages