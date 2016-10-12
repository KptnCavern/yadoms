﻿import unittest
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as Condition

   
def getShowDashboardButton(browser):
   return browser.find_element_by_id("btn-show-dashboard")
      
      
def get(browser):
   """ Return the dashboard WebElement """
   return browser.find_element_by_id("main-dashboard-modal")


def open(browser):
   """Open the dashboard and wait until displayed"""
   
   WebDriverWait(browser, 10).until(Condition.visibility_of_element_located((By.ID, "btn-show-dashboard")))
   getShowDashboardButton(browser).click()
   WebDriverWait(browser, 10).until(Condition.visibility_of_element_located((By.ID, "main-dashboard-modal")))
   return get(browser)


def openPage(browser, pageElementId, elementIdToWait):
   """Open a specific dashboard page"""
   
   pluginItem = browser.find_element_by_id(pageElementId)
   pluginItem.click()

   WebDriverWait(browser, 10).until(Condition.visibility_of_element_located((By.ID, elementIdToWait)))

   
def openAutomation(browser):
   """Open the automation page of dashboard"""
   
   openPage(browser, "btn-dashboard-automatisation", "automation-rule-list")

   
def openPlugin(browser):
   """Open the plugin page of dashboard"""
   
   openPage(browser, "btn-dashboard-plugins", "plugin-instance-list")

   
def openDevice(browser):
   """Open the device page of dashboard"""
   
   openPage(browser, "btn-dashboard-devices", "device-list")
   