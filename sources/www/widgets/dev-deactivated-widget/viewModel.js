widgetViewModelCtor =

/**
 * Create a DevDeactivatedWidget ViewModel
 * @constructor
 */
function DevDeactivatedWidgetViewModel() {
   /**
    * Initialization method
    * @param widget widget class object
    */
   this.initialize = function() {
      if (!isNullOrUndefined(this.widget.requiredType)) {
          this.widgetApi.find(".deactivatedWidget").text($.t("widgets/dev-deactivated-widget:deactivatedWithType", { widgetType: this.widget.requiredType }));
      }
      else {
          this.widgetApi.find(".deactivatedWidget").text($.t("widgets/dev-deactivated-widget:deactivated"));
      }
   };

   /**
    * Callback for a resized event
    */
   this.resized = function () {
      /*
      if (this.widget.width() <= 200) {
         this.widgetApi.find("div.deactivatedWidget span").css("font-size", "1em");
      }
      else
         this.widgetApi.find("div.deactivatedWidget span").css("font-size", "2em");
         */
   };
};
