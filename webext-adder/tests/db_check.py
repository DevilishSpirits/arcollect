import json
import sqlite3
import os

def open_db():
	return sqlite3.connect(os.environ['XDG_DATA_HOME']+'/arcollect/db.sqlite3')
## Tables on which to perform line count checks
# 
# This tuple list tables on which a line count check must be made. To pass the
# line count in the database must be the same as in the test_suite array.
db_line_count_checks = (
	'artworks',
	'accounts',
	'tags',
	'art_acc_links',
	'art_tag_links',
)
def linecount_checks(test_num, db, test_sets):
	# Accumulate counts
	counts = {}
	for table in db_line_count_checks:
		row_count = 0
		for test_set in test_sets:
			row_count += len(test_set[table])
		counts[table] = row_count
	
	# DB count checkings
	for table in db_line_count_checks:
		test_num += 1
		# FIXME That's an awful unsafe injection yet still with trusted data
		count_test = len(test_set[table])
		count_db   = db.execute('SELECT COUNT(*) FROM '+table+';').fetchone()[0]
		if count_db == count_test:
			ok_string = 'ok'
		else:
			ok_string = 'not ok'
		print(ok_string,test_num,'- Checking row count in',table,'table # Expected:',count_test,'Got:',count_db)

def test_count(test_set):
	#result  = len(db_line_count_checks)      # DB line count coherency checks
	result  = len(test_set['artworks'])      # DB artworks table check
	result += len(test_set['accounts'])      # DB accounts table check
	result += len(test_set['tags'])          # DB tags table check
	result += len(test_set['art_acc_links']) # DB art_acc_links table check
	result += len(test_set['art_tag_links']) # DB art_tag_links table check
	return result

def check_db(test_num, db, test_set):
	platform = test_set['platform']
	# Check artworks
	print('# Checking "artworks" table')
	for artwork in test_set['artworks']:
		test_num += 1
		artwork_db = db.execute('SELECT art_platform, art_title, art_desc, art_rating, art_height, art_width, art_mimetype, art_postdate FROM artworks where art_source = ?;',[artwork['source']]).fetchone()
		
		# Check if the artwork has been found
		if artwork_db is None:
			print('not ok',test_num,'- Checking artwork',artwork['source'],'# Not found in database')
		else:
			expected_values = (
				('art_platform',platform),
				('art_title'   ,artwork.setdefault('title',None) ),
				('art_desc'    ,artwork.setdefault('desc',None)  ),
				('art_rating'  ,artwork.setdefault('rating',None)),
				('art_height'  ,None                      ),
				('art_width'   ,None                      ),
				('art_mimetype',artwork.setdefault('mimetype','image/*')),
				('art_postdate',artwork.setdefault('postdate',None)),
			)
			mismatchs = []
			for i in range(len(expected_values)):
				expected = expected_values[i][1]
				got      = artwork_db[i]
				if expected != got:
					mismatchs.append(expected_values[i][0]+' mismatch (expected:'+str(expected)+', got:'+str(got)+')')
			
			if len(mismatchs) == 0:
				print('ok',test_num,'- Checking artwork',artwork['source'])
			else:
				print('not ok',test_num,'- Checking artwork',artwork['source'],'#',', '.join(mismatchs))
	
	# Check accounts
	print('# Checking "accounts" table')
	for account in test_set['accounts']:
		test_num += 1
		account_db = db.execute('SELECT acc_platform, acc_name, acc_title, acc_url FROM accounts where acc_platid = ?;',[account['id']]).fetchone()
		
		expected_values = (
			('acc_platform',platform),
			('acc_name'    ,account.setdefault('name' ,None)),
			('acc_title'   ,account.setdefault('title',None)),
			('acc_url'     ,account.setdefault('url',None)),
		)
		mismatchs = []
		for i in range(len(expected_values)):
			expected = expected_values[i][1]
			got      = account_db[i]
			if expected != got:
				mismatchs.append(expected_values[i][0]+' mismatch (expected:'+str(expected)+', got:'+str(got)+')')
		
		if len(mismatchs) == 0:
			print('ok',test_num,'- Checking account',account['id'])
		else:
			print('not ok',test_num,'- Checking account',account['id'],'#',', '.join(mismatchs))
	
	# Check tags
	print('# Checking "tags" table')
	for tag in test_set['tags']:
		test_num += 1
		tag_db = db.execute('SELECT tag_title, tag_kind FROM tags WHERE tag_platid = ? AND tag_platform = ?;',(tag['id'],platform)).fetchone()
		
		expected_values = (
			('tag_title'   ,tag.setdefault('title',None)),
			('tag_kind'    ,tag.setdefault('kind',None)),
		)
		mismatchs = []
		for i in range(len(expected_values)):
			expected = expected_values[i][1]
			got      = tag_db[i]
			if expected != got:
				mismatchs.append(expected_values[i][0]+' mismatch (expected:'+str(expected)+', got:'+str(got)+')')
		
		if len(mismatchs) == 0:
			print('ok',test_num,'- Checking tag',tag['id'])
		else:
			print('not ok',test_num,'- Checking tag',tag['id'],'#',', '.join(mismatchs))
	
	# Check art_acc_links
	print('# Checking "art_acc_links" table')
	for art_acc_link in test_set['art_acc_links']:
		test_num += 1
		count_in_db = db.execute('SELECT COUNT(*) FROM art_acc_links NATURAL JOIN artworks NATURAL JOIN accounts WHERE artacc_link = ? AND art_source = ? AND acc_platid = ?;',[art_acc_link['link'],art_acc_link['artwork'],art_acc_link['account']]).fetchone()[0]
		
		if count_in_db == 1:
			ok_string = 'ok'
		else:
			ok_string = 'not ok'
		
		print(ok_string,test_num,'- Checking',art_acc_link['artwork'],'to',art_acc_link['account'],art_acc_link['link'],'link # Found',count_in_db,'time(s)')
	
	# Check art_tag_links
	print('# Checking "art_tag_links" table')
	for art_tag_link in test_set['art_tag_links']:
		test_num += 1
		count_in_db = db.execute('SELECT COUNT(*) FROM art_tag_links NATURAL JOIN artworks NATURAL JOIN tags WHERE art_source = ? AND tag_platid = ?;',[art_tag_link['artwork'],art_tag_link['tag']]).fetchone()[0]
		
		if count_in_db == 1:
			ok_string = 'ok'
		else:
			ok_string = 'not ok'
		
		print(ok_string,test_num,'- Checking',art_tag_link['tag'],'tag on',art_tag_link['artwork'],'# Found',count_in_db,'time(s)')
	return test_num
