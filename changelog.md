### 2.0.2-rc.1

#### New features and fixes

##### Yadoms Core
* Filter some notifications
* Fix refresh-page option when disabled
* Fix navigation between dashboard pages
* Add icon on all dashboard pages

### 2.0.2-beta.4

#### New features and fixes

##### Yadoms Core
* Fix error notifications when updating Yadoms from web-client


### 2.0.2-beta.3

#### New features and fixes

##### Yadoms Core
* Same content as 2.0.2-beta.2 to test Yadoms update


### 2.0.2-beta.2

#### New features and fixes

##### Yadoms Core
* Fix update on RaspberryPi


### 2.0.2-beta.1

#### New features and fixes

##### Yadoms Core
* Refactor of install/update page
* Add changelog support for modules (plugins, widgets, script interpreters and Yadoms itself)
* Modules (plugins, widgets, script interpreters) can now specify a minimal Yadoms version (in their package.json)
* Webclient : By default, when refresh a page, the page reloaded is the current page/redesign configuration creation

##### Plugins
* RFXCom : Cartelectronic message fix TeleInfoStatus NoData to Enum Type
* TeleInfo : Add PEJP warning
* TeleInfo : historize runningPeriod and Forecast Tomorrow only when values changed

##### Widgets
* Thermometer : fix automatic display when return to default range (-40°C/50°C)
* Thermometer : better use of the size of the thermometer
* Thermostat : fix fire icon display
* Chart : fix limit precision for chart with range
* Chart : change the rotation off the label text of the axis x
* Chart : add enum keyword type support
* Gauge : fix flex and textfit
* Light : add support for curtain devices (SOMFY)
* Shutter : add the possibility to manipulate curtain
* Shutter : fix inverter function
* State-display : create this widget that could display enum/add all needed functions in widget-api

#### Issues fixed
* [Click here to see issue fixed in this version](https://github.com/Yadoms/yadoms/milestone/9?closed=1)


### 2.0.1

#### New features and fixes

##### Yadoms Core
* Graphical rules : fix dummy variables
* Graphical rules : fix function block
* Maintenance page : fix refresh
* Install & Update page : fix available versions
* Install & Update : fix update on RaspberryPi
* Fix sunrise/sunset computation (add timezone in system configuration page)

##### Plugins
N/A

##### Widgets
* Switch : add reverse state
* Chart : minimum decimal value is set to 0
* Chart : fix label
* Multicommand : fix usage of event keywords (without data)
* Bug fixes
* Click here to see issue fixed in this version

#### Bug fixes
* [Click here to see issue fixed in this version](https://github.com/Yadoms/yadoms/milestone/8?closed=1)


