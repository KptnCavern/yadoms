#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>

namespace yApi = shared::plugin::yPluginApi;

class IType
{
public:
   virtual ~IType()
   {
   }

public:
   virtual unsigned int id() const = 0;
   virtual const std::string& title() const = 0;
   virtual const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>& historizers() const = 0;
   virtual const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>& states(const boost::dynamic_bitset<>& data) const = 0;
};

