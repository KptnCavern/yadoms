#pragma once
#include <boost/msm/front/euml/euml.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>
#include <boost/msm/back/state_machine.hpp>

#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/Log.h>
#include <shared/event/EventTimer.h>
#include <shared/DataContainer.h>

#include "deviceStateCommonDeclaration.hpp"
#include "../WESConfiguration.h"
#include "../WESFactory.h"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

namespace msm = boost::msm;
using namespace boost::msm::front::euml;

//--------------------------------------------------------------
/// \brief	D�claration de l'�tat initialization
/// \note   
//--------------------------------------------------------------

BOOST_MSM_EUML_ACTION(Terminate_Exit)
{
   template <class Evt, class Fsm, class State>
   void operator()(Evt const& evt, Fsm& stateMachine, State&)
   {
      auto api = TerminateState.get_attribute(m_pluginApi);
   }
};

BOOST_MSM_EUML_TERMINATE_STATE((no_action, 
                                Terminate_Exit,
                                attributes_ << m_pluginApi,
                                configure_ << no_configure_),
                                TerminateState);