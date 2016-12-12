#pragma once
#include "Profile_D2_01_0D.h"

namespace yApi = shared::plugin::yPluginApi;


class CProfile_D2_01_0E : public CProfile_D2_01_0D
{
public:
   CProfile_D2_01_0E(const std::string& deviceId,
                     boost::shared_ptr<yApi::IYPluginApi> api);
   virtual ~CProfile_D2_01_0E();

protected:
   // CProfile_D2_01_0D override
   const std::string& profile() const override;
   const std::string& title() const override;
   // [END] CProfile_D2_01_0D override
};
