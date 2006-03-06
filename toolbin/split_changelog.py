#!/usr/bin/env python

#    Copyright (C) 2003 artofcode LLC.  All rights reserved.
#
# This file is part of GNU ghostscript
#
# GNU ghostscript is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2, or (at your option) any later version.
#
# This software is provided AS-IS with no warranty, either express or
# implied. That is, this program is distributed in the hope that it will 
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA, 02110-1301.


# $Id: split_changelog.py,v 1.5 2006/03/06 11:16:03 Arabidopsis Exp $

# script to generate the split Changes/Details html changelogs
# for Ghostscript from the output of 'cvs2cl.pl --xml'

import string, re
import xml.parsers.expat
import sys

class Entry:
	'''a class representing a single changelog entry'''
	data = {}
	has_details = False
	r = re.compile('^[ ]*DETAILS[ :]*$', re.I)
	c = re.compile('^[ ]*EXPECTED DIFFERENCES[ :]*$', re.I) 
	def reset(self):
		self.data = {}
		self.has_details = False
	def add(self, key, value):
		if not self.data.has_key(key): self.data[key] = value
		else: self.data[key] = string.join((self.data[key], value))
	def listadd(self, key, value):
		if not self.data.has_key(key): self.data[key] = [value]
		else: self.data[key].append(value)
	def addmsg(self, value):
		if not self.data.has_key('msg'): self.data['msg'] = []
		self.data['msg'].append(value)
		if self.r.search(value): self.has_details = True
		if self.c.search(value): self.has_details = True
	def write(self, file, details=True):
		stamp = self.data['date'] + ' ' + self.data['time']
		# construct the name anchor
		label = ''
		for c in stamp:
			if c == ' ': c = '_'
			if c == ':': continue
			label = label + c
		file.write('\n<p><strong>')
		file.write('<a name="' + label + '">')
		file.write('</a>\n')
		file.write(stamp + ' ' + self.data['author'])
		file.write('</strong>')
		if not details and self.has_details:
			file.write(' (<a href="' + details_fn + '#' + label + '">details</a>)')
		file.write('</p>\n')
		file.write('<blockquote>\n')
		file.write('<pre>\n')
		# todo: html-escape the msg lines
		for line in self.data['msg']:
			# skip the details unless wanted
			if not details and self.r.search(line): break
			if not details and self.c.search(line): break
			file.write(line)
		file.write('</pre>\n')
		file.write('<p>[')
		file.write(string.join(map(string.join, zip(self.data['name'],self.data['revision'])),', '))
		#file.write(string.join(self.data['name']))
		file.write(']</p>\n')
		file.write('</blockquote>\n')

def write_header(file, details=True):
	file.write('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"')
	file.write('   "http://www.w3.org/TR/html4/strict.dtd">\n')
	file.write('<html>\n')
	file.write('<head>\n')
	file.write('<title>')
	file.write('Ghostscript change history')
	if details:
		file.write(' (detailed)</title>')
	file.write('</title>\n')
	file.write('<!-- generated by split_changelog.py from the output of cvs2cl.pl -->\n')
	file.write('<!-- $Id: split_changelog.py,v 1.5 2006/03/06 11:16:03 Arabidopsis Exp $ -->\n')
	file.write('<link rel=stylesheet type="text/css" href="gs.css">\n')
	file.write('</head>\n')
	file.write('<body>\n')
	
def write_footer(file, details=True):
	file.write('</body>\n')
	file.write('</html>\n')

# expat hander functions
def start_element(name, attrs):
	#print 'Start element:', name, attrs
	element.append(name)
	if name == 'entry': e = Entry()
def end_element(name):
	#print 'End element:', name
	element.pop()
	if name == 'entry':
		e.write(details, True)
		e.write(changes, False)
		e.reset()
def char_data(data):
	if element[-1] == 'msg':
		# whitespace is meaningful inside the msg tag
		# so treat it specially
		e.addmsg(data)
	elif element[-1] == 'name' or element[-1] == 'revision':
		# keep an ordered list of these elements
		e.listadd(element[-1], string.strip(data))
	else:
		data = string.strip(data)
		if data:
			#print 'Character data:', data
			e.add(element[-1], data)
    

# global parsing state
element = []
e = Entry()

# create and hook up the expat instance
p = xml.parsers.expat.ParserCreate()
p.StartElementHandler = start_element
p.EndElementHandler = end_element
p.CharacterDataHandler = char_data

# open our files
if len(sys.argv) != 4:
	print 'usage: convert the output of cvs2cl --xml to html'
	print sys.argv[0] + ' <input xml> <output abbr> <output detailed>'
	sys.exit(2)
		
input = file(sys.argv[1], "r")
changes_fn = sys.argv[2]
details_fn = sys.argv[3]

changes = file(changes_fn, "w")
details = file(details_fn, "w")

# read the input xml and write the output html
write_header(changes, False)
write_header(details, True)

while 1:
	line = input.readline()
	if line == '': break
	p.Parse(line)

input.close()

write_footer(changes, False)
write_footer(details, True)

# finished
changes.close()
details.close()
