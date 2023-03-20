import time
import urllib.parse

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
	def __call__(self,artwork):
		# Find the 'Save in Arcollect' button by hrfe
		print("# Search 'Save in Arcollect' button")
		Save_in_Arcollect = None
		while Save_in_Arcollect is None:
			time.sleep(1)
			for button in self.webdriver.GetElements('css selector','button'):
				textContent = button.GetProperty('textContent').upper()
				if textContent == 'SAVE IN ARCOLLECT':
					print("# Found 'Save in Arcollect' button, clicking...")
					if button.Click().ok:
						Save_in_Arcollect = button
						break
					else:
						print('# Click failed likely due to a cookie overlay or obstruction')
				elif textContent.upper() == 'DISAGREE':
					# Click on the no cookies button
					print("# Found a 'Disagree' button, likely the cookie banner so click it")
					button.Click()
					break # Try again
				elif textContent.upper() == 'ACCEPT':
					# Click on the no cookies button
					print("# Found an 'Accept' button, likely the second cookie banner so click it")
					button.Click()
					break # Try again
		Save_in_Arcollect.Click()
		
		while Save_in_Arcollect.GetText() == 'Save in Arcollect':
			time.sleep(1)
		
		while Save_in_Arcollect.GetText() == 'Saving…':
			time.sleep(1)
		
		return Save_in_Arcollect.GetText() == 'Saved'
