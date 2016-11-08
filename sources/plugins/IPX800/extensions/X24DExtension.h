#pragma once

#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include "IExtension.h"
#include "../specificHistorizers/inputOutput.h"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

#define X24D_DI_QTY 24

namespace extensions
{
   //-----------------------------------------------------
   ///\brief X24-D Extension
   //-----------------------------------------------------
   class CX24DExtension : public IExtension
   {
   public:
      //-----------------------------------------------------
      ///\brief                     Constructor
      ///\param[in] keywordName     Yadoms keyword name
      //-----------------------------------------------------
      CX24DExtension(boost::shared_ptr<yApi::IYPluginApi> api,
                     const std::string& device,
                     const int& position);

      // IExtension implementation
      std::string getDeviceName() const override;
      int getSlot() const override;
      void updateFromDevice(boost::shared_ptr<yApi::IYPluginApi> api, shared::CDataContainer& values) const override;
      // [END] IExtension implementation

      //-----------------------------------------------------
      ///\brief                     Destructor
      //-----------------------------------------------------
      virtual ~CX24DExtension();

   private:

      //-----------------------------------------------------
      ///\brief                     The device name
      //-----------------------------------------------------
      std::string m_deviceName;

      //-----------------------------------------------------
      ///\brief                     The slot number
      //-----------------------------------------------------
      int m_slotNumber;

      //-----------------------------------------------------
      ///\brief                     The list of keyword
      //-----------------------------------------------------
      std::vector<boost::shared_ptr<specificHistorizers::CInputOuput> > m_keywordList;
   };
} // namespace extensions