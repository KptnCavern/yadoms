
/**
 * Create a Gauge ViewModel
 * @constructor
 */
widgetViewModelCtor = function gaugeViewModel() {

   /**
    * Initialization method
    * @param widget widget class object
    */

   this.initialize = function () {
      var self = this;

      //observable data
      this.data = ko.observable(0).extend({ numeric: 1 });
      this.unit = ko.observable("");

      //simple data
      this.WidgetHeight = "100px";
      this.WidgetWidth = "200px";

      this.isSmall = true;

      this.stopsArray = null;

      var d = new $.Deferred();
      self.widgetApi.loadLibrary([
          "libs/highstock/js/highstock.js",
          "libs/highstock/js/highcharts-more.js",
          "libs/highstock/js/modules/exporting.js",
          "libs/highstock/js/modules/solid-gauge.js",
          "libs/highcharts-export-clientside/js/highcharts-export-clientside.min.js"
      ]).done(function () {

         self.gaugeOptions = {

            chart: {
               type: "solidgauge"
            },

            title: null,
            pane: {
               center: ["50%", "85%"],
               size: "140%",
               startAngle: -90,
               endAngle: 90,
               background: {
                  backgroundColor: (Highcharts.theme && Highcharts.theme.background2) || "#EEE",
                  innerRadius: "60%",
                  outerRadius: "100%",
                  shape: "arc"
               }
            },

            tooltip: {
               enabled: false
            },
            credits: {
               enabled: false
            },

            // the value axis
            yAxis: {
               //            stops are not defined here
               lineWidth: 0,
               minorTickInterval: null,
               tickPixelInterval: 400,
               tickWidth: 0,
               title: {
                  y: -70
               },
               labels: {
                  enabled: true,
                  align: "left",
                  x: -5,
                  y: 16
               }
            },

            plotOptions: {
               solidgauge: {
                  dataLabels: {
                     enabled: true,
                     y: 5,
                     borderWidth: 0,
                     useHTML: true
                  }
               }
            },

            exporting: {
               enabled: false
            }
         };

         //we create the battery indicator
         self.widgetApi.toolbar.addBatteryIconToWidget();

         d.resolve();
      });
      return d.promise();
   };

   /**
   * New acquisition handler
   * @param keywordId keywordId on which new acquisition was received
   * @param data Acquisition data
   */
   this.onNewAcquisition = function (keywordId, data) {
      var self = this;

      if (keywordId === self.widget.configuration.device.keywordId) {
         if (data.value !== self.data()) // refresh only when it's necessary.
         {
            //it is the good device
            self.data(data.value);
            self.refreshValue();
         }
      }
   };

   this.refreshValue = function () {
      var self = this;

      var element = self.widgetApi.find(".widget-gauge-main");

      var chart = element.highcharts();

      if (chart) {
         if (chart.series && chart.series.length > 0) {
            if (chart.series[0].points && chart.series[0].points.length > 0) {
               var point = chart.series[0].points[0];
               point.update(self.data());
            }
         }
      }
   };

   this.configurationChanged = function () {
      var self = this;

      if ((isNullOrUndefined(this.widget)) || (isNullOrUndefinedOrEmpty(this.widget.configuration)))
         return;

      //we register keyword new acquisition
      self.widgetApi.registerKeywordAcquisitions(self.widget.configuration.device.keywordId);

      // Delete all elements in stopArray
      this.stopsArray = new Array();

      //we fill the deviceId of the battery indicator
      self.widgetApi.toolbar.configureBatteryIcon(self.widget.configuration.device.deviceId);


      switch (self.widget.configuration.displayMode.activeSection) {
         case "solidColor":
            self.stopsArray.push([0, self.widget.configuration.displayMode.content.solidColor.content.color]);
            break;
         case "thresholds":
            var previousColor = self.widget.configuration.displayMode.content.thresholds.content.firstColor;
            self.widget.configuration.displayMode.content.thresholds.content.addedThresholds.forEach(function (addedThreshold) {
               var thresholdRatio = (addedThreshold.content.value - self.widget.configuration.customYAxisMinMax.content.minimumValue) /
               (self.widget.configuration.customYAxisMinMax.content.maximumValue - self.widget.configuration.customYAxisMinMax.content.minimumValue);

               self.stopsArray.push([thresholdRatio - 0.001, previousColor]);
               self.stopsArray.push([thresholdRatio, addedThreshold.content.color]);

               previousColor = addedThreshold.content.color;
            });
            break;
         case "gradient":
            self.stopsArray.push([0, self.widget.configuration.displayMode.content.gradient.content.minColor]);
            self.stopsArray.push([1, self.widget.configuration.displayMode.content.gradient.content.maxColor]);
            break;
      }

      //we get the unit of the keyword
      KeywordManager.get(self.widget.configuration.device.keywordId)
      .done(function (keyword) {
         self.unit($.t(keyword.units));
         //The following function should be here, otherwise, the unit, is not up to date
         self.refresh();
         self.refreshValue();
      });
   };

   this.resized = function () {
      // The square size is not supported by the widget. So size only used here 1x2 and 2x3.

      var self = this;

      //The maximum size
      if (this.widget.height() <= 220 && this.widget.height() >= 180 && this.widget.width() <= 320 && this.widget.width() >= 280) {
         //self.WidgetWidth  = "277px";
         //self.WidgetHeight = "150px";
         self.WidgetWidth = "277px";
         self.WidgetHeight = "170px";

         //To be painted only one time
         if (self.isSmall === true) {
            self.isSmall = false;
            self.refresh();
            self.refreshValue();
         }
      }
      else {
         self.WidgetWidth = "193px";
         self.WidgetHeight = "130px";

         //To be painted only one time
         if (self.isSmall === false) {
            self.isSmall = true;
            self.refresh();
            self.refreshValue();
         }
      }
   };

   this.refresh = function () {
      var self = this;

      var sizeValue;
      var sizeUnit;
      var titlePosition;

      if (self.isSmall === true) {
         sizeValue = 12;
         sizeUnit = 8;
         titlePosition = -33;
      }
      else {
         sizeValue = 24;
         sizeUnit = 12;
         titlePosition = -70;
      }

      var element = self.widgetApi.find(".widget-gauge-main");

      //Attributes of div could only be changed trough theses variables. In an other way the div is stretched.	   
      element.css("height", self.WidgetHeight);
      element.css("width", self.WidgetWidth);

      //we configured the chart !
      element.highcharts(Highcharts.merge(self.gaugeOptions, {
         navigation: {
            buttonOptions: {
               enabled: !self.isSmall
            }
         },
         yAxis: {
            min: parseInt(self.widget.configuration.customYAxisMinMax.content.minimumValue), //Minimum value
            max: parseInt(self.widget.configuration.customYAxisMinMax.content.maximumValue), //Maximal value
            stops: self.stopsArray, // Stops for bar colors with thresholds
            title: {
               enabled: "middle",
               text: self.widget.configuration.text,
               y: titlePosition
            }
         },

         series: [{
            name: 'Data',
            data: [1],
            dataLabels: {
               format: '<div style="text-align:center"><span style="text-align:center;font-size:' + sizeValue + 'px;color:' +
                        ((Highcharts.theme && Highcharts.theme.contrastTextColor) || 'black') + '">{y:.1f}</span>' +  //<br/>
                           '<span style="text-align:center;font-size:' + sizeUnit + 'px;color:silver"> ' + self.unit() + '</span></div>'
            },
            tooltip: {
               valueSuffix: self.unit()
            }
         }]
      }));
   };
};