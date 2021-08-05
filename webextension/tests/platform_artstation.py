import time
import urllib.parse

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
	def __call__(self):
		# Note: 'Save in Arcollect' buttons are not visible in WebDriver until shown
		#        by a mouse move over artworks.
		
		# Find the right 'Download' button by hrfe
		# The 'Save in Arcollect' button is near
		download_url_substr = urllib.parse.unquote(self.webdriver.GetCurrentURL().split('#')[1].split('&')[0].replace('_','?'))
		aBtn_elements = self.webdriver.GetElements('css selector','a.btn')
		print("# Search 'Download' with",download_url_substr,"in href")
		for aBtn in aBtn_elements:
			href = urllib.parse.unquote(aBtn.GetProperty('href'))
			if href.find(download_url_substr) >= 0:
				print("# Found 'Download' element with href:",href)
				# Scroll to the rect
				#self.webdriver.post('actions',{"actions":[{'id': "Hein?!", 'type': 'wheel', 'actions': [{'type': 'scroll', 'origin': 'viewport', 'x': 0, 'y': aBtn.GetRect()}]}]})
				self.webdriver.post('execute/sync',{'script': 'window.scrollBy(0,'+str(aBtn.GetRect()['y'])+')','args':[]})
				break
			else:
				aBtn = None
		if aBtn is None:
			print("# No matching 'Download' element found!")
			return False
		
		Save_in_Arcollect = []
		while len(Save_in_Arcollect) != 1:
			time.sleep(1)
			aBtn_rect = aBtn.GetRect()
			self.webdriver.post('actions',{"actions":[{'id': "hein?!", 'type': 'pointer', 'actions': [{'type': 'pointerMove', 'origin': 'viewport', 'x': int(aBtn_rect['x']), 'y': 250}]}]})
			Save_in_Arcollect = self.webdriver.GetElements('link text','Save in Arcollect')
		Save_in_Arcollect = Save_in_Arcollect[0]
		Save_in_Arcollect.Click()
		
		while Save_in_Arcollect.GetText() == 'Save in Arcollect':
			time.sleep(1)
		
		while Save_in_Arcollect.GetText() == 'Saving...':
			time.sleep(1)
		
		return Save_in_Arcollect.GetText() == 'Saved'
