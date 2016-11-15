#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <boost/dynamic_bitset.hpp>
#include "../IType.h"

namespace yApi = shared::plugin::yPluginApi;


class CProfile_D2_01_0F : public IType
{
public:
   CProfile_D2_01_0F();
   virtual ~CProfile_D2_01_0F();
public:
   unsigned int id() const override;
   const std::string& title() const override;
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> allHistorizers() const override;
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> states(const boost::dynamic_bitset<>& data) const override;
private:
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> m_historizers;
};

