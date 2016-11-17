#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <boost/dynamic_bitset.hpp>
#include "../IType.h"

namespace yApi = shared::plugin::yPluginApi;


class CProfile_D2_01_12 : public IType
{
public:
   DECLARE_ENUM_HEADER(EDefaultState,
      ((off)(0))
      ((on)(1))
      ((previousState)(2))
      ((notUsed)(3))
   );

   DECLARE_ENUM_HEADER(EConnectedSwitchsType,
      ((switch)(0))
      ((pushButton)(1))
      ((autodetection)(2))
   );


   CProfile_D2_01_12();
   virtual ~CProfile_D2_01_12();

   // IType implementation
   const std::string& profile() const override;
   const std::string& title() const override;
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> allHistorizers() const override;
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> states(const boost::dynamic_bitset<>& data) const override;
   void sendCommand(const std::string& keyword,
                    const std::string& commandBody) const override;
   void sendConfiguration(const shared::CDataContainer& deviceConfiguration) const override;
   // [END] IType implementation

private:
   boost::shared_ptr<yApi::historization::CSwitch> m_switch1;
   boost::shared_ptr<yApi::historization::CSwitch> m_switch2;
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> m_historizers;
};
