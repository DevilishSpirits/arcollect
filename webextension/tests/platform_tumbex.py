from random import randint
import time
import urllib.parse

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
	def __call__(self,artwork):
		# Find and click on the share button
		shareButton = []
		while len(shareButton) == 0:
			shareButton = self.webdriver.GetElements('css selector','#post-container .dropdown-toggle')
			time.sleep(1)
		shareButton[0].Click()
		
		Save_in_Arcollect = self.webdriver.GetElements('link text','SAVE IN ARCOLLECT')[0]
		Save_in_Arcollect.Click()
		
		Saving_in_Arcollect = self.webdriver.GetElements('css selector','#post-container .btn-group-sm > .btn > small')[0]
		while Saving_in_Arcollect.GetText() == 'Saving…':
			time.sleep(1)
		
		return Saving_in_Arcollect.GetText() == 'Saved'
