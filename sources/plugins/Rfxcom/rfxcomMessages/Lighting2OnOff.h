#pragma once

#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include "ILighting2Subtype.h"

namespace yApi = shared::plugin::yPluginApi;

namespace rfxcomMessages
{
   //--------------------------------------------------------------
   /// \brief	The Lighting2 On/off keyword
   //--------------------------------------------------------------
   class CLighting2OnOff : public ILighting2Subtype
   {
   public:
      //--------------------------------------------------------------
      /// \brief	                        Constructor
      /// \param[in] model                The device model
      //--------------------------------------------------------------
      explicit CLighting2OnOff(const std::string& model);

      //--------------------------------------------------------------
      /// \brief	Destructor
      //--------------------------------------------------------------
      virtual ~CLighting2OnOff();

      // ILighting2Subtype implementation
      std::string getModel() const override;
      boost::shared_ptr<const yApi::historization::IHistorizable> keyword() const override;
      void set(const std::string& yadomsCommand) override;
      void reset() override;
      void idFromProtocol(unsigned char id1Byte,
                          unsigned char id2Byte,
                          unsigned char id3Byte,
                          unsigned char id4Byte,
                          unsigned char& houseCode,
                          unsigned int& id) const override;
      void idToProtocol(unsigned char houseCode,
                        unsigned int id,
                        unsigned char& id1Byte,
                        unsigned char& id2Byte,
                        unsigned char& id3Byte,
                        unsigned char& id4Byte) const override;
      void setFromProtocolState(unsigned char cmdByte,
                                unsigned char levelByte) override;
      void toProtocolState(unsigned char& cmdByte,
                           unsigned char& levelByte) const override;
      // [END] ILighting2Subtype implementation

   private:
      //--------------------------------------------------------------
      /// \brief	                        The device model
      //--------------------------------------------------------------
      const std::string m_model;

      //--------------------------------------------------------------
      /// \brief	                        The keyword
      //--------------------------------------------------------------
      boost::shared_ptr<yApi::historization::CSwitch> m_keyword;
   };
} // namespace rfxcomMessages


