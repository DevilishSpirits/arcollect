import time

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
		# Click on the  to use cookies 🍪️
		print('# Agree that the user is over 18+')
		self.webdriver.navigateTo('https://www.e621.net/posts?tags=rating%3Asafe+status%3Aactive') # TODO Check success
		self.webdriver.GetElement('css selector','button#guest-warning-accept').Click()
	def __call__(self,artwork):
		# Get the 'Save in Arcollect' button
		Save_in_Arcollect = self.webdriver.GetElement('link text','Save in Arcollect')
		Save_in_Arcollect.Click()
		
		while Save_in_Arcollect.GetText() == 'Save in Arcollect':
			time.sleep(1)
		
		while Save_in_Arcollect.GetText() == 'Saving…':
			time.sleep(1)
		
		return Save_in_Arcollect.GetText() == 'Saved'
