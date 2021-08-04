""" @file WebDriver.py
    @brief WebDriver implementation
	
	This file implement a WebDriver wrapper (https://w3c.github.io/webdriver/).
"""
import requests
import json
import os

## Web-extension ZIP path
# 
# This convenience constant is filled with the absolute version of
# `$ARCOLLECT_WEBEXT_ZIP_PATH` environment variable that contain the path to the
# unsigned web-extension ZIP file in build tree.
extension_path = os.path.abspath(os.environ['ARCOLLECT_WEBEXT_ZIP_PATH'])

def assert_request_or_bailout(req, test_number, test_name, bailout_message):
	""" Assert that a request succeded, print TAP line and 'Bail Out!' on failure
		@param req             The Requests object
		@param test_number     The test number to display
		@param test_name       The test name to display
		@param bailout_message Message to display on 'Bail Out!' line
		@return req            The req param
		
		This function is a convenience assertion utility. It print a TAP line test
		and upon failure it TAP 'Bail Out!' and raise an exception.
	"""
	if req.status_code == requests.codes.ok:
		print('ok',test_number,'-',test_name)
	else:
		print('not ok',test_number,'-',test_name)
		print('Bail Out!',bailout_message)
		req.raise_for_status()
	return req

class WebElement:
	def __init__(self,webdriver,element_id):
		self.webdriver = webdriver
		self.element_id = element_id
		self.base_url = 'element/'+self.element_id+'/'
	def get(self, url, data = {}, **kwargs):
		return self.webdriver.get(self.base_url+url,data,**kwargs)
	def post(self, url, data = {}, **kwargs):
		return self.webdriver.post(self.base_url+url,data,**kwargs)
	
	def Click(self):
		return self.post('click')
	def GetText(self):
		return self.get('text').json()['value']

class WebDriver:
	def direct_request(self, method, url, data = {}, **kwargs):
		url = self.base_url+url
		print('#',method,url,data)
		req = self.webdriver.request(method,url,data=json.dumps(data),**kwargs)
		print('#',method,'\t-> ',req.status_code,req.text)
		return req
	def request(self, method, url, data = {}, **kwargs):
		return self.direct_request(method, 'session/'+self.session_id+'/'+url, data, **kwargs)
	
	def get(self, url, data = {}, **kwargs):
		return self.request('GET',url,data,**kwargs)
	def post(self, url, data = {}, **kwargs):
		return self.request('POST',url,data,**kwargs)
	def delete(self, url, data = {}, **kwargs):
		return self.request('DELETE',url,data,**kwargs)
	
	def __init__(self,test_num,base_url,session_start_data):
		self.session_id = None
		self.webdriver = requests.session()
		self.base_url = base_url
		
		# Fire POST "/session"
		req = assert_request_or_bailout(self.direct_request('POST','session', session_start_data),test_num,'Create WebDriver session','Session creation failed')
		self.wd_session_results = req.json()
		self.session_id = self.wd_session_results['value']['sessionId']
		print('# Session id:',self.session_id)
	
	def __del__(self):
		if self.session_id != None:
			self.delete('window')
	
	def navigateTo(self,url):
		return self.post('url',{"url": url})
	def GetElement(self,using,value):
		result = self.post('element',{"using": using, 'value': value}).json()['value']
		for key in result:
			return WebElement(self,result[key])
	def GetElements(self,using,value):
		results = self.post('elements',{"using": using, 'value': value}).json()['value']
		elements = []
		for result in results:
			for key in result:
				elements.append(WebElement(self,result[key]))
		return elements
