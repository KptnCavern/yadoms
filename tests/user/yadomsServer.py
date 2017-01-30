﻿import subprocess
import signal
import time
import os
import os.path
import psutil
import platform
from selenium.webdriver.support.ui import WebDriverWait

def binaryPath():
   """return the server binary path"""
   
   if (platform.system() == "Windows"):
      return os.path.join("..", "..", "builds", "DEBUG")
   else:
      return os.path.join("..", "..", "builds")
   
   
def databasePath():
   """return the server database path"""
   
   return os.path.join(binaryPath(), "yadoms.db3")
   
   
def logsPath():
   """return the logs path"""
   
   return os.path.join(binaryPath(), "logs")
   
   
def pluginLogsPath():
   """return the plugin log path"""
   
   return os.path.join(logsPath(), "plugins")
   
   
def scriptLogsPath():
   """return the script log path"""
   
   return os.path.join(logsPath(), "scripts")
   
   
def scriptsPath():
   """return the scripts path"""
   
   return os.path.join(binaryPath(), "scripts")
   
   
def pluginsPath():
   """return the plugins path"""
   
   return os.path.join(binaryPath(), "plugins")
   
   
def wwwLocalesPath():
   """return the locales path of the web client"""
   
   return os.path.join(binaryPath(), "www", "locales")
   
   
def start(startupArgs=[]):
   """Start the Yadoms server"""
   
   # Set logLevel to none if not provided
   if not startupArgs or all("logLevel=" not in arg for arg in startupArgs):
      startupArgs.append("logLevel=none")

   argsLine = ""
   for arg in startupArgs:
      argsLine += " --" + arg

   print 'Start server...'
   executableName = "yadoms.exe" if platform.system() == "Windows" else "yadoms"
   return subprocess.Popen("python yadomsServerWrapper.py " + os.path.join(binaryPath(), executableName + " " + argsLine), stdin=subprocess.PIPE)
   
   
def isProcessRunning(pid):  
   """Check if the process is running"""

   if (platform.system() == "Windows"):
      try:
         os.kill(pid, 0)
      except OSError as err:
         if err.errno == errno.ESRCH:
            return False
      return True
   else:
      return os.path.exists("/proc/" + str(pid))
         
      
def killProcTree(pid, including_parent=True):
   """Kill a parent process with its children"""
   parent = psutil.Process(pid)
   children = parent.children(recursive=True)
   while parent.children(recursive=True):
      try:
         parent.children(recursive=True)[0].kill()
      except:
         pass
   psutil.wait_procs(parent.children(recursive=True), timeout=5)
   if including_parent:
      try:
         parent.kill()
         parent.wait(20)
         print 'Process killed'
      except:
         print 'Error : process still alive'
         assert False
      
  
def stop(yadomsProcess):
   """Kill Yadoms server with its sub-processes"""

   print 'Stop Yadoms...'
   try:
      yadomsProcess.communicate(input=' ')  
   except KeyboardInterrupt:
      print 'Stopped'
      return
   except Exception, e:
      print 'Error stopping Yadoms', str(e)

   print 'Yadoms was not gracefully stopped, try to kill...'
   killProcTree(yadomsProcess.pid)

           
def restart():
   """Restart the Yadoms server"""
   stop()
   start()


def waitServerStarted():
   import datetime
   import requests
   timeout = datetime.datetime.now() + datetime.timedelta(seconds = 30)
   while datetime.datetime.now() < timeout:
      try:
         response = requests.post('http://127.0.0.1:8080/rest/general/system', timeout=1)
         if response.status_code == requests.codes.ok:
            print 'Server started'
            return
      except:
        pass

   print 'Server not responding'
   assert False


def openClient(browser):
   """Open a client on local server and wait for full loading"""
   waitServerStarted()
   try:
      browser.get("http://127.0.0.1:8080")
      if WebDriverWait(browser, 20).until(lambda driver: driver.execute_script("return document.readyState") == u"complete" and driver.execute_script("return jQuery.active") == 0):
         return
   except:
      print 'Exception waiting page loding'

   print 'Unable to load client'
   assert False
