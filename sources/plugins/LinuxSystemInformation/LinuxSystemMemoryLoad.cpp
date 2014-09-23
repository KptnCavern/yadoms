#include "stdafx.h"
#include "LinuxSystemMemoryLoad.h"
#include <shared/exception/Exception.hpp>
#include <shared/plugin/yadomsApi/StandardCapacities.h>
#include <shared/plugin/yadomsApi/StandardUnits.h>

#include "sys/types.h"
//TODO : Temp
#include <shared/Log.h>

// Shortcut to yadomsApi namespace
namespace yApi = shared::plugin::yadomsApi;

CLinuxSystemMemoryLoad::CLinuxSystemMemoryLoad(const std::string & device)
   :m_device(device), m_keyword("MemoryLoad")
{}

CLinuxSystemMemoryLoad::~CLinuxSystemMemoryLoad()
{}

void CLinuxSystemMemoryLoad::declareKeywords(boost::shared_ptr<yApi::IYadomsApi> context)
{
   context->declareKeyword(m_device, m_keyword);
}

void CLinuxSystemMemoryLoad::historizeData(boost::shared_ptr<yApi::IYadomsApi> context) const
{
   BOOST_ASSERT_MSG(!!context, "context must be defined");

   context->historizeData(m_device, m_keyword);
}

void CLinuxSystemMemoryLoad::read()
{
   if (sysinfo (&memInfo)!=0)
   {
      std::stringstream Message; 
      Message << "sysinfo failed !"; 
      throw shared::exception::CException ( Message.str() );
   }

   long long totalVirtualMem = memInfo.totalram;

   totalVirtualMem += memInfo.totalswap;
   totalVirtualMem *= memInfo.mem_unit;

   long long virtualMemUsed = memInfo.totalram - memInfo.freeram;

   YADOMS_LOG(debug) << "Mémoire virtuelle utilisée :" << virtualMemUsed;
   YADOMS_LOG(debug) << "Mémoire virtuelle totale   :" << totalVirtualMem;

   //FIXME : Cette méthode renvoie une valeur supérieure à ce que me renvoie le moniteur système d'Ubuntu ... A vérifier. Domoticz donne la meme chose. A vérifier avec une autre fonction mémoire en ligne de commande.

   m_keyword.set( virtualMemUsed*100/double(totalVirtualMem));

   YADOMS_LOG(debug) << "WindowsSystemInformation plugin :  Memory Load : " << m_keyword.formatValue();
}


