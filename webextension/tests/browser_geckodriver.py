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
		appmanifest_dest = os.environ['HOME']+'/'+os.environ['FIREFOX_APPMANIFEST_DEST']
		os.makedirs(os.path.dirname(appmanifest_dest),exist_ok=True)
		os.unlink(appmanifest_dest)
		os.link(os.environ['FIREFOX_APPMANIFEST_SOURCE'],appmanifest_dest)
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
		self.webdriver = WebDriver.WebDriver(1,'http://127.0.0.1:4444/',session_start_data)
		# Install addon
		WebDriver.assert_request_or_bailout(self.webdriver.post('moz/addon/install', {'path': WebDriver.extension_path,'temporary': True}),2,'Install Arcollect addon','Failed to install Arcollect addon!')
		# Install uBlock Origin
		if 'UBLOCK_ORIGIN_XPI_PATH' in os.environ:
			WebDriver.assert_request_or_bailout(self.webdriver.post('moz/addon/install', {'path': os.environ['UBLOCK_ORIGIN_XPI_PATH']}),3,'Install uBlock Origin','Failed to install uBlock Origin addon!')
		else:
			print('ok 3 - Install uBlock Origin # SKIP $UBLOCK_ORIGIN_XPI_PATH is not set')
	
	def __del__(self):
		self.webdriver = None # Force WebDriver destruction
		self.geckodriver.kill()
		self.geckodriver.wait()
