#pragma once
#include "IDevice.h"
#include "IIdentification.h"
#include "../ioInterfaces/IQuadAdConverter.h"
#include <shared/plugin/yPluginApi/historization/IHistorizable.h>

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

namespace device
{
   //--------------------------------------------------------------
   /// \brief	4-AD converters (Family 20)
   //--------------------------------------------------------------
   class CQuadAdConverter : public IDevice
   {
   public:
      //--------------------------------------------------------------
      /// \brief	Constructor
      /// \param[in]	family Device family
      /// \param[in]	id Device serial number
      /// \param[in]	io I/O access object
      //--------------------------------------------------------------
      CQuadAdConverter(EOneWireFamily family,
                       const std::string& id,
                       boost::shared_ptr<ioInterfaces::IQuadAdConverter> io);

      //--------------------------------------------------------------
      /// \brief	Destructor
      //--------------------------------------------------------------
      virtual ~CQuadAdConverter();

   protected:
      // IDevice implementation
      boost::shared_ptr<const IIdentification> ident() const override
      {
         return m_identification;
      }
      const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >& keywords() const override
      {
         return m_keywords;
      }
      void get() const override;
      void set(const std::string& keyword, const std::string& command) override;
      // [END] IDevice implementation

   private:
      //--------------------------------------------------------------
      /// \brief	The AD Converter number
      //--------------------------------------------------------------
      enum { NbAdConverter = 4 };

      //--------------------------------------------------------------
      /// \brief	The device identification
      //--------------------------------------------------------------
      boost::shared_ptr<const IIdentification> m_identification;

      //--------------------------------------------------------------
      /// \brief	The yApi context
      //--------------------------------------------------------------
      boost::shared_ptr<yApi::IYPluginApi> m_context;

      //--------------------------------------------------------------
      /// \brief	The I/O access object
      //--------------------------------------------------------------
      boost::shared_ptr<ioInterfaces::IQuadAdConverter> m_io;

      //--------------------------------------------------------------
      /// \brief	The keywords
      //--------------------------------------------------------------
      boost::shared_ptr<yApi::historization::CVoltage> m_keyword[NbAdConverter];
      std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_keywords;
   };
} // namespace device