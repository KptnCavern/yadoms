#pragma once
#include "IRfxcomMessage.h"
#include "RFXtrxHelpers.h"
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/DataContainer.h>
#include "IRemoteSubtype.h"

namespace yApi = shared::plugin::yPluginApi;

namespace rfxcomMessages
{
   //--------------------------------------------------------------
   /// \brief	The Remote protocol support
   //--------------------------------------------------------------
   class CRemote : public IRfxcomMessage
   {
   public:
      //--------------------------------------------------------------
      /// \brief	                        Constructor
      /// \param[in] api                  Yadoms APi context
      /// \param[in] command              The command
      /// \param[in] deviceDetails        The device parameters
      /// \throw                          shared::exception::CInvalidParameter if fail to interpret command
      /// \note                           Use this constructor for command (to build RFXCom message)
      //--------------------------------------------------------------
      CRemote(boost::shared_ptr<yApi::IYPluginApi> api,
              const std::string& command,
              const shared::CDataContainer& deviceDetails);

      //--------------------------------------------------------------
      /// \brief	                        Constructor
      /// \param[in] api                  Yadoms APi context
      /// \param[in] rbuf                 The received buffer
      /// \param[in] rbufSize             Message size, received from Rfxcom
      /// \note                           Use this constructor for received messages (to historize received data to Yadoms)
      /// \throw                          shared::exception::CInvalidParameter
      //--------------------------------------------------------------
      CRemote(boost::shared_ptr<yApi::IYPluginApi> api,
              const RBUF& rbuf,
              size_t rbufSize);

      //--------------------------------------------------------------
      /// \brief	Destructor
      //--------------------------------------------------------------
      virtual ~CRemote();

      // IRfxcomMessage implementation
      boost::shared_ptr<std::queue<shared::communication::CByteBuffer> > encode(boost::shared_ptr<ISequenceNumber> seqNumberProvider) const override;
      void historizeData(boost::shared_ptr<yApi::IYPluginApi> api) const override;
      const std::string& getDeviceName() const override;
      const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>& keywords() override;
      // [END] IRfxcomMessage implementation

   protected:
      //--------------------------------------------------------------
      /// \brief	Set and create the subtype
      /// \param[in] subType              Device subType
      //--------------------------------------------------------------
      void createSubType(unsigned char subType);

      //--------------------------------------------------------------
      /// \brief	                        Build the device name
      //--------------------------------------------------------------
      void buildDeviceName();

      //--------------------------------------------------------------
      /// \brief	Declare the device
      /// \param[in] api                  Yadoms APi context
      //--------------------------------------------------------------
      void declare(boost::shared_ptr<yApi::IYPluginApi> api);

   private:
      //--------------------------------------------------------------
      /// \brief	The device sub-type
      //--------------------------------------------------------------
      unsigned char m_subType;

      //--------------------------------------------------------------
      /// \brief	The remote id
      //--------------------------------------------------------------
      unsigned int m_id;

      //--------------------------------------------------------------
      /// \brief	The device name
      //--------------------------------------------------------------
      std::string m_deviceName;

      //--------------------------------------------------------------
      /// \brief	The sub-type management
      //--------------------------------------------------------------
      boost::shared_ptr<IRemoteSubtype> m_subTypeManager;

      //--------------------------------------------------------------
      /// \brief	The signalPower (percent)
      //--------------------------------------------------------------
      boost::shared_ptr<yApi::historization::CSignalPower> m_signalPower;

      //--------------------------------------------------------------
      /// \brief	The keywords list to historize in one step for better performances
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_keywords;
   };
} // namespace rfxcomMessages


