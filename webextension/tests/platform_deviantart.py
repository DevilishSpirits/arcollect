import time
import urllib.parse

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
	def __call__(self):
		# Find the 'Save in Arcollect' button by hrfe
		button_elements = self.webdriver.GetElements('css selector','button')
		print("# Search 'Save in Arcollect' button")
		Save_in_Arcollect = None
		for button in button_elements:
			if button.GetProperty('textContent') == 'Save in Arcollect':
				print("# Found 'Save in Arcollect' button")
				Save_in_Arcollect = button
				break
		Save_in_Arcollect.Click()
		
		while Save_in_Arcollect.GetText() == 'Save in Arcollect':
			time.sleep(1)
		
		while Save_in_Arcollect.GetText() == 'Saving...':
			time.sleep(1)
		
		return Save_in_Arcollect.GetText() == 'Saved'
