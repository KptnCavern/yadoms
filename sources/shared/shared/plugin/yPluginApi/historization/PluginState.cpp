#include "stdafx.h"
#include "PluginState.h"
#include "../StandardValues.h"
#include "../StandardCapacity.h"
#include "../StandardUnits.h"
#include <shared/exception/InvalidParameter.hpp>
#include <shared/DataContainer.h>
#include <shared/StringExtension.h>


namespace shared { namespace plugin { namespace yPluginApi { namespace historization
{

DECLARE_ENUM_IMPLEMENTATION(EPluginState,
   ((Unknown))
   ((Error))
   ((Stopped))
   ((Running))
   ((Custom))
);

const shared::plugin::yPluginApi::CStandardCapacity& PluginStateCapacity = shared::plugin::yPluginApi::CStandardCapacity("pluginState_capacity", shared::plugin::yPluginApi::CStandardUnits::NoUnits, shared::plugin::yPluginApi::EKeywordDataType::kEnum);

CPluginState::CPluginState(const std::string& keywordName, const EKeywordAccessMode& accessMode)
   :CSingleHistorizableData<EPluginState>(keywordName, PluginStateCapacity, accessMode)
{
}

CPluginState::~CPluginState()
{
}

} } } } // namespace shared::plugin::yPluginApi::historization

