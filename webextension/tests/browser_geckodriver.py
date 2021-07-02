from subprocess import Popen,DEVNULL
import time
import sys
import os
import WebDriver
import requests

class Browser:
	def test_num():
		return 3
	def __init__(self):
		# Install manifest
		print('not ok 1 - Install native-messaging manifest # TODO Install manifest logic')
		# Spawn geckodriver
		# TODO Do not close stdout
		self.geckodriver = Popen(['geckodriver'],stdout=DEVNULL)
		time.sleep(1) # FIXME please FIXME
		# Initialize WebDriver session
		session_start_data = {
			"capabilities": {
				"alwaysMatch": {
					"browserName": "firefox",
					"browserVersion": ">=52"
				}
			}
		}
		self.webdriver = WebDriver.WebDriver(2,'http://127.0.0.1:4444/',session_start_data)
		# Install addon
		WebDriver.assert_request_or_bailout(self.webdriver.post('moz/addon/install', {'path': WebDriver.extension_path,'temporary': True}),3,'Install Arcollect addon','Failed to install Arcollect addon!')
	
	def __del__(self):
		self.webdriver = None # Force WebDriver destruction
		self.geckodriver.kill()
		self.geckodriver.wait()
