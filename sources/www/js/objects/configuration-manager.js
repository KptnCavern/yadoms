/**
 * Created by nicolasHILAIRE on 25/02/14.
 */

/**
 * Ctor which does nothing because it is used as a static class
 * @constructor
 */
function ConfigurationManager(){}

/**
 * Create a ConfigurationItem from json content
 * @param {Json} json The configuration item as json
 * @returns {ConfigurationItem} The newly created configuration item
 */
ConfigurationManager.factory = function(json) {
   assert(!isNullOrUndefined(json), "json must be defined");
   assert(!isNullOrUndefined(json.section), "json.section must be defined");
   assert(!isNullOrUndefined(json.name), "json.name must be defined");
   assert(!isNullOrUndefined(json.value), "json.value must be defined");
   assert(!isNullOrUndefined(json.defaultValue), "json.defaultValue must be defined");
   assert(!isNullOrUndefined(json.description), "json.description must be defined");
   assert(!isNullOrUndefined(json.securityAccess), "json.securityAccess must be defined");
   assert(!isNullOrUndefined(json.lastModificationDate), "json.lastModificationDate must be defined");

   return new ConfigurationItem(json.section, json.name, ConfigurationItem.decodeValue(json.value),
       ConfigurationItem.decodeValue(json.defaultValue), json.description, json.securityAccess, json.lastModificationDate);
};

//Here is the list of items of system configuration
ConfigurationManager.items = {};
ConfigurationManager.items.system = {};
ConfigurationManager.items.systemSection = "system";
ConfigurationManager.items.system.language = "language";
ConfigurationManager.items.system.timezone = "timezone";

ConfigurationManager.items.system.basicAuthentication = "basicAuthentication";
ConfigurationManager.items.system.basicAuthenticationUser = "basicAuthenticationUser";
ConfigurationManager.items.system.basicAuthenticationPassword = "basicAuthenticationPassword";
ConfigurationManager.items.system.basicAuthenticationPassword2 = "basicAuthenticationPassword2";

ConfigurationManager.items.system.location = {};
ConfigurationManager.items.system.locationSection = "location";
ConfigurationManager.items.system.location.latitude  = "latitude";
ConfigurationManager.items.system.location.longitude = "longitude";
ConfigurationManager.items.system.location.altitude  = "altitude";

ConfigurationManager.items.system.advancedParameters = "advancedParameters";
ConfigurationManager.items.system.dateFormatString = "dateFormatString";
ConfigurationManager.items.system.refreshPage = "refreshPage";

ConfigurationManager.items.install = {};
ConfigurationManager.items.installSection = "install";
ConfigurationManager.items.install.firstStart = "firstStart";

/**
 * Get all configuration values from a section
 * @param {Section} section The section
 * @returns {Promise} 
 */
ConfigurationManager.getSection = function(section) {
   assert(!isNullOrUndefined(section), "section must be defined");

   var d = new $.Deferred();

   RestEngine.getJson("rest/configuration/" + section)
      .done(function(data) {
         var result = [];

         $.each(data.configuration, function (index, value) {
            var ci = ConfigurationManager.factory(value);
            result[ci.name] = ci;
         });

         d.resolve(result);
      })
      .fail(d.reject);

   return d.promise();
};

/**
 * Get a configuration value from server
 * @param {String} configurationSection The item section
 * @param {String} configurationName The item key
 * @returns {Promise} 
 */
ConfigurationManager.get = function(configurationSection, configurationName) {
   assert(!isNullOrUndefined(configurationSection), "configurationSection must be defined");
   assert(!isNullOrUndefined(configurationName), "configurationName must be defined");

   var d = new $.Deferred();
   RestEngine.getJson("rest/configuration/" + configurationSection + "/" + configurationName)
      .done(function(data) {
         var result = ConfigurationManager.factory(data);
         d.resolve(result);
      })
      .fail(d.reject);
   return d.promise();
};


/**
 * Update a configuration item
 * @param {Object} configurationItem The configuration item to update
 * @returns {Promise} 
 */
ConfigurationManager.updateToServer = function (configurationItem) {
   assert(!isNullOrUndefined(configurationItem), "configurationItem must be defined");
   return RestEngine.putJson("/rest/configuration/" + configurationItem.section + "/" + configurationItem.name, { data: JSON.stringify(configurationItem) });
};


/**
 * Save a configuration item 
 * @param {String} section The configuration item section
 * @param {String} name The configuration item identifier
 * @param {String} value The value 
 * @param {String} defaultValue The default value
 * @param {String} description The descrption
 * @param {String} securityAccess The securit access
 * @returns {Promise} 
 */
ConfigurationManager.createToServer = function(section, name, value, defaultValue, description, securityAccess) {
   assert(!isNullOrUndefined(section), "section must be defined");
   assert(!isNullOrUndefined(name), "name must be defined");
   assert(!isNullOrUndefined(value), "value must be defined");

   if (isNullOrUndefined(securityAccess))
      securityAccess = "none";

   if (isNullOrUndefined(defaultValue))
      defaultValue = value;
   
   return RestEngine.putJson("/rest/configuration/" + section + "/" + name, {
      data: JSON.stringify({
         "section": section,
         "name": name,
         "value": value,
         "defaultValue": defaultValue,
         "description": description,
         "securityAccess": securityAccess
      })
   });
};

ConfigurationManager.saveRefreshPageDefaultValue = function (arrayOfDeffered) {
   var deffered = ConfigurationManager.createToServer(ConfigurationManager.items.systemSection, ConfigurationManager.items.system.refreshPage, true, true, "", null);
   
   deffered
      .done(function() {
          console.info("refresh page : " + "true");
      })
     .fail(function(error) {
         notifyError($.t("objects.ConfigurationManager.errorDuringGettingConfiguration", {
             section: ConfigurationManager.items.systemSection,
             key: ConfigurationManager.items.system.refreshPage
         }), error);
     });
     
     Yadoms.systemConfiguration[ConfigurationManager.items.system.refreshPage] = true;
     arrayOfDeffered.push(deffered);
};

ConfigurationManager.saveDefaultAdvancedParameters = function (arrayOfDeffered){
  var deffered = ConfigurationManager.createToServer(ConfigurationManager.items.systemSection, ConfigurationManager.items.system.advancedParameters, false);
  
  deffered
     .done(function() {
        console.info("Advanced parameters : false");
     })
     .fail(function(error) {
        notifyError($.t("objects.ConfigurationManager.errorDuringGettingConfiguration", {
            section: ConfigurationManager.items.systemSection,
            key: ConfigurationManager.items.system.advancedParameters
        }), error);
     });
  arrayOfDeffered.push(deffered);
};

ConfigurationManager.saveDefaultDateFormatString = function (arrayOfDeffered) {
  var deffered = ConfigurationManager.createToServer(ConfigurationManager.items.systemSection, ConfigurationManager.items.system.dateFormatString, "LLL");
  
  deffered
      .done(function() {
          console.info("Date Format String : " + "LLL");
      })
      .fail(function(error) {
          notifyError($.t("objects.ConfigurationManager.errorDuringGettingConfiguration", {
              section: ConfigurationManager.items.systemSection,
              key: ConfigurationManager.items.system.dateFormatString
          }), error);
      }); 
   arrayOfDeffered.push(deffered);
};

ConfigurationManager.saveDefaultBasicAuthentification = function (arrayOfDeffered) {
   var basicAuth = {
      "active": false,
      "user": "admin",
      "password": ""
   };
   
   var deffered = ConfigurationManager.createToServer(ConfigurationManager.items.systemSection, ConfigurationManager.items.system.basicAuthentication, JSON.stringify(basicAuth));
   
   deffered
        .done(function() {
            console.info("Basic authentication : " + JSON.stringify(basicAuth));
        })
        .fail(function(error) {
            notifyError($.t("objects.ConfigurationManager.errorDuringGettingConfiguration", {
                section: ConfigurationManager.items.systemSection,
                key: ConfigurationManager.items.system.basicAuthentication
            }), error);
        });
   arrayOfDeffered.push(deffered);
};

ConfigurationManager.saveDefaultLanguage = function (arrayOfDeffered, defaultLanguage) {
    var deffered = ConfigurationManager.createToServer(ConfigurationManager.items.systemSection, ConfigurationManager.items.system.language, defaultLanguage, "", "language used by default");
    
    deffered
        .done(function() {
            console.info("Language detected : " + defaultLanguage);
        })                                 
        .fail(function(error) {
            notifyError($.t("objects.ConfigurationManager.errorDuringGettingConfiguration", {
                section: ConfigurationManager.items.systemSection,
                key: ConfigurationManager.items.system.language
            }), error);
        }); 
     arrayOfDeffered.push(deffered);   
};

ConfigurationManager.saveFirstStart = function (arrayOfDeffered) {
   var deffered = ConfigurationManager.createToServer(ConfigurationManager.items.installSection, ConfigurationManager.items.install.firstStart, "false", "true", "First start of Web app has been done");
   
   deffered
    .fail(function(error) {
        notifyError($.t("objects.ConfigurationManager.errorDuringGettingConfiguration", {
            section: ConfigurationManager.items.installSection,
            key: ConfigurationManager.items.install.firstStart
        }), error);
    });
    arrayOfDeffered.push(deffered);   
};