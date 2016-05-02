#pragma once

#include "ITransceiver.h"
#include <shared/communication/IAsyncPort.h>
#include "ITeleInfoMessage.h"
#include "TeleInfotrxHelpers.h"
#include "ISequenceNumberProvider.h"
#include "Keywords/RunningPeriod.h"
#include "Keywords/ForecastTomorrow.h"

// Shortcut to yadomsApi namespace
namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	This class implement the TeleInfo protocol
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
   virtual void decodeTeleInfoMessage(boost::shared_ptr<yApi::IYPluginApi> context,
	                                  std::string & PluginName,
                                      const shared::communication::CByteBuffer& data);
   virtual bool IsInformationUpdated ( void );
   virtual void ResetRefreshTags ( void );
   // [END] ITransceiver implementation

private:
   //--------------------------------------------------------------
   /// \brief  The message sequence number
   //--------------------------------------------------------------
   boost::shared_ptr<ISequenceNumberProvider> m_seqNumberProvider;

   //--------------------------------------------------------------
   /// \brief	                     calculate et check the checksum of the message
   /// \param [in] m_buffer          Buffer of the message
   /// \return                       true if the checksum is valid
   //--------------------------------------------------------------

   bool isCheckSumOk(const unsigned char *m_buffer) const;

   //--------------------------------------------------------------
   /// \brief	                     Parse the message received
   /// \param [in] pData             Pointer to the buffer
   /// \param [in] Len               Length of the buffer
   //--------------------------------------------------------------

   void ParseData( const unsigned char *pData, int Len);

   //--------------------------------------------------------------
   /// \brief	                     Identify the message selected
   /// \param [in] m_buffer          The buffer that contains the message
   /// \param [in] context           YADOMS API
   /// \param [in] PluginName        The plugin Name
   //--------------------------------------------------------------

   void MatchLine( const unsigned char* m_buffer );

   //--------------------------------------------------------------
   /// \brief	                     Historize TeleInfoData
   /// \param [in] KeywordName       The keyword Name
   /// \param [in] Value             The value of the data
   //--------------------------------------------------------------

   template <class T>
   void HistorizeTeleInfoData ( std::string KeywordName, long Value );

   //--------------------------------------------------------------
   /// \brief	                     Create the Device with the counter Id
   /// \param [in] CounterId         The counter Id
   //--------------------------------------------------------------
   void CreateDevice ( std::string CounterId );

   //--------------------------------------------------------------
   /// \brief  Keywords list
   //--------------------------------------------------------------

   std::vector<boost::shared_ptr<yApi::historization::IHistorizable> > m_KeywordList;

   //--------------------------------------------------------------
   /// \brief  Details of keywords
   //--------------------------------------------------------------

   shared::CDataContainer m_details;

   //--------------------------------------------------------------
   /// \brief  Keywords
   //--------------------------------------------------------------

	boost::shared_ptr<CRunningPeriod> m_TimePeriod;
	boost::shared_ptr<CForecastTomorrow> m_ForecastPeriod;

	boost::shared_ptr<yApi::IYPluginApi> m_context;
	std::string m_PluginName;

	//Contract Options -> Abonnement
	typedef enum {
	   OP_NOT_DEFINED,
	   OP_BASE,
	   OP_CREUSE,
	   OP_EJP,
	   OP_TEMPO
	} Abonnement;

	bool baseUpdated;
	bool LowCostUpdated;
	bool NormalCostUpdated;
	bool InstantCurrentUpdated;
	bool ApparentPowerUpdated;

	bool EJPPeakPeriodUpdated;
	bool EJPNormalPeriodUpdated;

	bool TempoBlueDaysLowCostUpdated;
	bool TempoBlueDaysNormalCostUpdated;

	bool TempoWhiteDaysLowCostUpdated;
	bool TempoWhiteDaysNormalCostUpdated;

	bool TempoRedDaysLowCostUpdated;
	bool TempoRedDaysNormalCostUpdated;

	bool TimePeriodUpdated;
	bool ForecastPeriodUpdated;

	bool m_deviceCreated;

	Abonnement Optarif;
};
