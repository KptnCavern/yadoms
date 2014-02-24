#pragma once

#include "entities/Entities.h"


namespace database { 

   class IWidgetRequester
   {
   public:
      //--------------------------------------------------------------
      /// \brief           Add new widget 
      /// \param [in]      newWidget   New widget informations
      /// \return          widgetId generated by the database
      /// \throw           shared::exception::CEmptyResult if fails
      //--------------------------------------------------------------
      virtual int addWidget(const entities::CWidget & newWidget) = 0;

      //--------------------------------------------------------------
      /// \brief                 Get widget informations
      /// \param [in] widgetId   Widget Id
      /// \throw                 CInvalidParameter if widgetId is unknown
      //--------------------------------------------------------------
      virtual boost::shared_ptr<entities::CWidget> getWidget(int widgetId) = 0;

      //--------------------------------------------------------------
      /// \brief           List all widgets
      /// \return          List of registered widgets
      //--------------------------------------------------------------
      virtual std::vector<boost::shared_ptr<entities::CWidget> > getWidgets() = 0;

      //--------------------------------------------------------------
      /// \brief           List all widgets instanciated in a page
      /// \param [in]      pageId   page  Id
      /// \return          List of registered widgets
      //--------------------------------------------------------------
      virtual std::vector<boost::shared_ptr<entities::CWidget> > getWidgetsForPage(int pageId) = 0;
      
      //--------------------------------------------------------------
      /// \brief           Update widget informations
      /// \param [in]      widgetId   Widget  Id
      /// \param [in]      newConfiguration   New configuration
      /// \throw           shared::exception::CEmptyResult if fails
      //--------------------------------------------------------------
      virtual void updateWidgetConfiguration(int widgetId, const std::string& newConfiguration) = 0;

      //--------------------------------------------------------------
      /// \brief           Update widget size
      /// \param [in]      widgetId   Widget  Id
      /// \param [in]      sizeX   the size X
      /// \param [in]      sizeX   the size Y
      /// \throw           shared::exception::CEmptyResult if fails
      //--------------------------------------------------------------
      virtual void updateWidgetSize(int widgetId, int sizeX, int sizeY) = 0;
      
      //--------------------------------------------------------------
      /// \brief           Remove widget 
      /// \param [in] widgetId   Widget  Id
      /// \throw           shared::exception::CEmptyResult if fails
      //--------------------------------------------------------------
      virtual void removeWidget(int widgetId) = 0;

      //--------------------------------------------------------------
      /// \brief           Remove all widgets conained in a page
      /// \param [in] widgetId   the page id
      /// \throw           shared::exception::CEmptyResult if fails
      //--------------------------------------------------------------
      virtual void removeWidgetsInPage(int pageId) = 0;

      //--------------------------------------------------------------
      /// \brief           Remove all widgets
      /// \throw           shared::exception::CEmptyResult if fails
      //--------------------------------------------------------------
      virtual void removeAllWidgets() = 0;

      //--------------------------------------------------------------
      /// \brief       Destructor
      //--------------------------------------------------------------
      virtual ~IWidgetRequester()
      {
      }
   };
 
} //namespace database 
