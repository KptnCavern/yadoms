#include "stdafx.h"
#include "Type0State.h"
#include <shared/plugin/yPluginApi/StandardCapacities.h>
#include <shared/plugin/yPluginApi/StandardUnits.h>

namespace specificHistorizers {
      
   DECLARE_ENUM_IMPLEMENTATION(EType0StateValues,
      ((Off)("OFF"))
      ((On)("ON"))
      ((Bright)("BRIGHT"))
      ((Dim)("DIM"))
      ((AllOff)("ALL_OFF"))
      ((AllOn)("ALL_ON"))
   )

   DECLARE_CAPACITY(Type0StateCapacity, "x10state", yApi::CStandardUnits::NoUnits, yApi::EKeywordDataType::kEnum);

   CType0State::CType0State(const std::string& keywordName)
      : CSingleHistorizableData<EType0StateValues>(keywordName, Type0StateCapacity, yApi::EKeywordAccessMode::kGetSet)
   {
   }

   CType0State::~CType0State()
   {
   }

} //namespace specificHistorizers