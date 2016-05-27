#pragma once
#include "IDevice.h"
#include "IIdentification.h"
#include "../ioInterfaces/ITemperature.h"
#include <shared/plugin/yPluginApi/historization/IHistorizable.h>

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

namespace device
{
   //--------------------------------------------------------------
   /// \brief	Common class for single temperature devices
   //--------------------------------------------------------------
   class CSingleTemperature : public IDevice
   {
   public:
      //--------------------------------------------------------------
      /// \brief	Constructor
      /// \param[in]	family Device family
      /// \param[in]	id Device serial number
      /// \param[in]	model Device model (chip reference)
      /// \param[in]	io I/O access object
      /// \param[in]	expectedFamily Expected family (used for runtime check)
      //--------------------------------------------------------------
      CSingleTemperature(EOneWireFamily family,
                         const std::string& id,
                         const std::string& model,
                         boost::shared_ptr<ioInterfaces::ITemperature> io,
                         EOneWireFamily expectedFamily);

      //--------------------------------------------------------------
      /// \brief	Destructor
      //--------------------------------------------------------------
      virtual ~CSingleTemperature();

      //--------------------------------------------------------------
      /// \brief	Check if read temperature is valid
      /// \param[in] temperature Temperature value to check
      /// \return true if temperature is valid
      //--------------------------------------------------------------
      static bool isTemperatureValid(double temperature);

   protected:
      // IDevice implementation
      boost::shared_ptr<const IIdentification> ident() const override
      {
         return m_identification;
      }
      const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >& keywords() const override
      {
         return *m_keywords;
      }
      void get() const override;
      void set(const std::string& keyword, const std::string& command) override;
      // [END] IDevice implementation

   private:
      //--------------------------------------------------------------
      /// \brief	The device identification
      //--------------------------------------------------------------
      boost::shared_ptr<const IIdentification> m_identification;

      //--------------------------------------------------------------
      /// \brief	The I/O access object
      //--------------------------------------------------------------
      boost::shared_ptr<ioInterfaces::ITemperature> m_io;

      //--------------------------------------------------------------
      /// \brief	The temperature (�C)
      //--------------------------------------------------------------
      yApi::historization::CTemperature m_temperature;

      std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >* m_keywords;
      const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_allKeywords;
      const std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_noKeywords;
   };
} // namespace device