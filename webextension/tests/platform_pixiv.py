import time

class Platform:
	def __init__(self,webdriver):
		self.webdriver = webdriver
	def __call__(self,artwork):
		# Get and click on the 'Save in Arcollect' button
		Save_in_Arcollect = []
		while len(Save_in_Arcollect) == 0:
			print('# Looking for the "Save in Arcollect" span button')
			Save_in_Arcollect = list(filter(lambda span: span.GetText() == 'Save in Arcollect', self.webdriver.GetElements('css selector','span')))
			time.sleep(1)
		print('# Found',len(Save_in_Arcollect),'!')
		Save_in_Arcollect = Save_in_Arcollect[0]
		# We need to manually scroll on the button
		while not Save_in_Arcollect.Click().ok:
			y = str(Save_in_Arcollect.GetRect()['y'])
			self.webdriver.post('execute/sync',{'script': 'window.scrollBy(0,-'+y+'0)','args':[]}) # Back scroll to avoid overshootings
			self.webdriver.post('execute/sync',{'script': 'window.scrollBy(0,'+y+')','args':[]})
		
		while Save_in_Arcollect.GetText() == 'Save in Arcollect':
			time.sleep(1)
		
		while Save_in_Arcollect.GetText() == 'Saving…':
			time.sleep(1)
		
		return Save_in_Arcollect.GetText() == 'Saved'
