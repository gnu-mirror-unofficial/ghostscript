#!/usr/bin/env python

#    Copyright (C) 2001-2004 Artifex Software Inc.
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


# $Id: gscheck_raster.py,v 1.5 2006/03/06 11:16:03 Arabidopsis Exp $

#
# gscheck_raster.py
#
# rasterizes input files in several configurations and checks them
# against known baselines
#


import os, stat
import string, calendar, time
import gstestutils
import gssum, gsconf, gstestgs, gsparamsets, gsutil

class GSCompareTestCase(gstestgs.GhostscriptTestCase):
    def shortDescription(self):
        file = "%s.%s.%d.%d" % (self.file[string.rindex(self.file, '/') + 1:], self.device, self.dpi, self.band)
	rasterfilename = gsconf.rasterdbdir + file + ".gz"
	if not os.access(rasterfilename, os.F_OK):
		os.system(gsconf.codedir + "update_baseline '%s'" %
                          (os.path.basename(self.file),))
	try:
		ct = time.localtime(os.stat(rasterfilename)[stat.ST_MTIME])
		baseline_date = "%s %d, %4d %02d:%02d" % ( calendar.month_abbr[ct[1]], ct[2], ct[0], ct[3], ct[4] )
	except:
		if self.band:
			banded = "banded"
		else:
			banded = "noband"
		return "Skipping %s (%s/%ddpi/%s) [no previous raster data found]" % (os.path.basename(self.file), self.device, self.dpi, banded)

	if self.band:
	    return "Checking %s (%s/%ddpi/banded) against baseline set on %s" % (os.path.basename(self.file), self.device, self.dpi, baseline_date)
        else:
	    return "Checking %s (%s/%ddpi/noband) against baseline set on %s" % (os.path.basename(self.file), self.device, self.dpi, baseline_date)

    def runTest(self):
	file = "%s.%s.%d.%d" % (self.file[string.rindex(self.file, '/') + 1:], self.device, self.dpi, self.band)

	gs = gstestgs.Ghostscript()
	gs.command = self.gs
	gs.device = self.device
	gs.dpi = self.dpi
	gs.band = self.band
	gs.infile = self.file
	gs.outfile = file
	if self.log_stdout:
	    gs.log_stdout = self.log_stdout
	if self.log_stderr:
	    gs.log_stderr = self.log_stderr

	if gs.process():
	    sum = gssum.make_sum(file)
        else:
	    sum = ''
	os.unlink(file)

	# add test result to daily database
	if self.track_daily:
	    gssum.add_file(file, dbname=gsconf.get_dailydb_name(), sum=sum)

	if not sum:
	    self.fail("output file could not be created "\
		      "for file: " + self.file)
        else:
	    self.assertEqual(sum, gssum.get_sum(file),
			     'md5sum did not match baseline (' +
			     file + ') for file: ' + self.file)


# add compare tests
def add_compare_test(suite, f, device, dpi, band, track):
    suite.addTest(GSCompareTestCase(gs=gsconf.comparegs,
                                    file=gsconf.comparefiledir + f,
                                    device=device, dpi=dpi,
                                    band=band,
                                    log_stdout=gsconf.log_stdout,
                                    log_stderr=gsconf.log_stderr,
                                    track_daily=track))


def addTests(suite, gsroot, **args):
    if args.has_key('track'):
        track = args['track']
    else:
        track = 0

    # get a list of test files
    comparefiles = os.listdir(gsconf.comparefiledir)

    for f in comparefiles:
        if gsutil.check_extension(f):
	    for params in gsparamsets.testparamsets:
	        add_compare_test(suite, f, params.device,
				 params.resolution, params.banding, track)


if __name__ == '__main__':
    gstestutils.gsRunTestsMain(addTests)

