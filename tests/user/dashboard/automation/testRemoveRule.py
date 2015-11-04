﻿import unittest
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support.ui import Select
from selenium.webdriver.support import expected_conditions as Condition
from selenium.webdriver.common.keys import Keys
import database
import scripts
import yadomsServer
import dashboard
import dashboard.automation
import notification
import i18n
import tools

class RemoveRule(unittest.TestCase):
   """Remove rule test"""
   
   def setUp(self):
      database.withOneStoppedRule()
      scripts.deploy(["DisplayLatitude"])
      self.serverProcess = yadomsServer.start()
      self.browser = webdriver.Firefox()
      yadomsServer.openClient(self.browser)
      
      
      
   def initialConditionsForRemoveStoppedRuleTest(self, rulesTable, ruleNumber):
      assert dashboard.automation.getRuleState(rulesTable, ruleNumber) is dashboard.automation.RuleState.Stopped
   
   def test_removeStoppedRule(self):
      self.doTest_removeRule(lambda rulesTable, ruleNumber: self.initialConditionsForRemoveStoppedRuleTest(rulesTable, ruleNumber))
      
      
   def initialConditionsForRemoveRunningRuleTest(self, rulesTable, ruleNumber):
      dashboard.automation.getRuleStartStopButton(rulesTable, ruleNumber)
      tools.waitUntil(lambda: dashboard.automation.getRuleState(rulesTable, ruleNumber) is dashboard.automation.RuleState.Running)
   
   def test_removeRunningRule(self):
      self.doTest_removeRule(lambda rulesTable, ruleNumber: self.initialConditionsForRemoveRunningRuleTest(rulesTable, ruleNumber))
      
      
   def doTest_removeRule(self, initialConditionsFct):
      # Open rules dashboard
      dashboard.open(self.browser)
      dashboard.openAutomation(self.browser)
      ruleNumber = 0

      # Get rule table
      rulesTable = dashboard.automation.waitRulesTableHasNRules(self.browser, 1)
      removeButton = dashboard.automation.getRuleRemoveButton(rulesTable, ruleNumber)
      
      initialConditionsFct(rulesTable, ruleNumber)
      
      # Remove rule
      removeButton.click()
      confirmationModal = dashboard.automation.waitRuleRemovingConfirmationModal(self.browser)
      dashboard.automation.getRuleRemovingConfirmationModalOkButton(confirmationModal).click()
      
      # Notification expected
      notification.wait(self.browser, notification.Type.Success, i18n.get()["modals"]["dashboard"]["sub-windows"]["automation-center"]["ruleDeleted"])
      # Table should be updated
      assert tools.waitUntil(lambda: dashboard.automation.getRuleNumberInTable(self.browser, rulesTable) == 0, 5)
  
      
      
   def test_dontConfirmRemoveRule(self):
      # Open rules dashboard
      dashboard.open(self.browser)
      dashboard.openAutomation(self.browser)
      ruleNumber = 0

      # Get rule table
      rulesTable = dashboard.automation.waitRulesTableHasNRules(self.browser, 1)
      removeButton = dashboard.automation.getRuleRemoveButton(rulesTable, ruleNumber)
      
      assert dashboard.automation.getRuleState(rulesTable, ruleNumber) is dashboard.automation.RuleState.Stopped
      
      # Remove rule
      removeButton.click()
      confirmationModal = dashboard.automation.waitRuleRemovingConfirmationModal(self.browser)
      dashboard.automation.getRuleRemovingConfirmationModalCancelButton(confirmationModal).click()
      
      # No notification expected
      assert notification.noNotification(self.browser)
      # No change in rule table    
      assert dashboard.automation.getRuleNumberInTable(self.browser, rulesTable) is 1
      
      
   def tearDown(self):
      self.browser.close()
      yadomsServer.stop(self.serverProcess)

if __name__ == "__main__":
   unittest.main()