import time

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
	def __call__(self):
		# Get the 'Save in Arcollect' button
		Save_in_Arcollect = self.webdriver.GetElement('link text','Save in Arcollect')
		Save_in_Arcollect.Click()
		
		while Save_in_Arcollect.GetText() == 'Save in Arcollect':
			time.sleep(1)
		
		while Save_in_Arcollect.GetText() == 'Saving…':
			time.sleep(1)
		
		return Save_in_Arcollect.GetText() == 'Saved'
