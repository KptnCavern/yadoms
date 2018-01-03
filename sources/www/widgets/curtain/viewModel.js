widgetViewModelCtor =

   /**
    * Create a Curtain ViewModel
    * @constructor
    */
      function curtainViewModel() {
          //observable data
          this.command = ko.observable(1);
          this.capacity = [];
          this.accessMode = [];
          this.showDeviceName = ko.observable(true);
		  
          /**
           * method to send command to Yadoms
           * @param newState to send
           */        
          this.commandClick = function (newState) {

              var self = this;

              if  (!isNullOrUndefined(self.widget.configuration)) {
                  // Checks for the first device
                  if (!isNullOrUndefined(self.widget.configuration.device))
                  {
                     if (!isNullOrUndefined(self.capacity[0])) {
                        // We send the command only for Set and GetSet variables
                        if ( self.accessMode[0] === "GetSet" || self.accessMode[0] === "Set" )
                           KeywordManager.sendCommand(self.widget.configuration.device.keywordId, newState.toString());
                      }
                  }                   
                  
                  if (!isNullOrUndefined(self.widget.configuration.additionalDevices.content.devices)){
                     // Check for the others devices
                     $.each(self.widget.configuration.additionalDevices.content.devices, function (index, device) {
                    
                         if (!isNullOrUndefined(self.capacity[index+1])) {
                             // We send the command only for Set and GetSet variables
                             if ( self.accessMode[index+1] === "GetSet" || self.accessMode[index+1] === "Set" )
                                KeywordManager.sendCommand(device.content.source.keywordId, newState.toString());
                         }
                     });
                  }
              }
          };

          /**
           * Initialization method
           * @param widget widget class object
           */
          this.initialize = function () {
             //we configure the toolbar
             this.widgetApi.toolbar({
                 activated: true,
                 displayTitle: true,
                 batteryItem: false
             });
          };

          /**
           * ConfigurationChanged method
           * @param none
           */          
          this.configurationChanged = function () {
              var self = this;
              var readOnlyMode=false;
              var arrayOfDeffered = [];
              var d = new $.Deferred();

              if ((isNullOrUndefined(self.widget)) || (isNullOrUndefinedOrEmpty(self.widget.configuration)))
                  return;

              // keep the information if the device name should be shown
              if (!isNullOrUndefined(self.widget.configuration.showDeviceName)) {
                  self.showDeviceName(parseBool(self.widget.configuration.showDeviceName));
              }
              
               //we ask for the first device information
              if  (!isNullOrUndefined(this.widget.configuration.device.deviceId)) {
                // Get the capacity of the keyword
                var deffered = KeywordManager.get(self.widget.configuration.device.keywordId);
                
                deffered
                .done(function (keyword) {
                  self.capacity[0]   = keyword.capacityName;
                  self.accessMode[0] = keyword.accessMode;
                });
                
                arrayOfDeffered.push(deffered);
              }
              
              //we ask for additional devices information
              if ((!isNullOrUndefined(self.widget.configuration.additionalDevices.content.devices))) {
                  $.each(self.widget.configuration.additionalDevices.content.devices, function (index, device) {
                      if (!isNullOrUndefined(device.content.source.deviceId)) {
						  
                         // Get the capacity of the keyword
                         var deffered = KeywordManager.get(device.content.source.keywordId);
                         
                         deffered
                         .done(function (keyword) {
                           self.capacity[index+1]   = keyword.capacityName;
                           self.accessMode[index+1] = keyword.accessMode;
                         });
                         
                         arrayOfDeffered.push(deffered);
                      }
                  });
               }              
             // This variable is used only for the display
             $.when.apply($, arrayOfDeffered)
             .done(function () {
                d.resolve();
             })
             .fail(function () {
                d.reject();
             });
            
             return d.promise();                
          };        
      };