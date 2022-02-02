#!/usr/bin/env python

# Author: Kris Nikov - kris.nikov@bris.ac.uk
# Date: 22 Dec 2019
# Description:  Plotting script for ARMPM using matplotlip

import re
import sys
import getopt
import numpy as np
import math
import matplotlib.pyplot as plt
import pylab
from matplotlib.backends.backend_pdf import PdfPages
# import matplotlib.patches as mpatches
# from operator import itemgetter
# import matplotlib.lines as mlines
# from matplotlib.legend_handler import HandlerPatch
# from matplotlib.legend_handler import HandlerLine2D
from matplotlib.pyplot import cm
from matplotlib import rc#, rcParams
import copy


# class ArrowHandler(HandlerLine2D):
	# def create_artists(self, legend, orig_handle, xdescent, ydescent, width, height, fontsize, trans):
		# xdata, xdata_marker = self.get_xdata(legend, xdescent, ydescent, width, height, fontsize)
		# ydata = ((height - ydescent) / 2.) * np.ones(xdata.shape, float)
		# legline = mpatches.FancyArrow(xdata[0], ydata[0], xdata[1]-5, 0, width=1, head_length=5, head_width=5, color=orig_handle.get_color())
		# #self.update_prop(legline, orig_handle, legend)
		# legline.set_transform(trans)
		# return [legline]

# def make_legend_arrow(legend, orig_handle, xdescent, ydescent, width, height, fontsize):
	# legline = mpatches.FancyArrow(0, 0.5*height, width, 0, length_includes_head=True, head_length=5, head_width=5)
	# return legline

# def f(): raise Exception("Found exit()")

def main(argv):
	plottype = 0
	xlabel = ''
	xticks=0
	ylabel = ''
	legendloc=-1
	bfile = ''
	blabel = ''
	inputfile = [] #use array for input file
	inputfile_header = ''
	inputlabel = []
	inputdir = []
	outputfile = ''

	energy_flag = 0
	sample = []
	phys_value = []
	pred_value = []
	abs_err = []
	rel_err = []

	bfile_header = ''
	inputfile_header = ''

	#rc('text', usetex=True)
	rc('font', weight='bold')
	rc('font', size='25')
	rc('axes', labelsize='35')
	#rc('legend', fontsize='11')	# legend fontsize
	#rcParams['text.latex.preamble'] = [r'\usepackage{sfmath} \boldmath']
	my_dpi = 96

	try:
		opts, args = getopt.getopt(argv, "hp:x:t:y:e:b:l:i:a:o:", ["ptype=", "xlabel=", "xticks=", "ylabel=", "legendloc=", "bfile=", "blabel=", "ifile=", "ilabel=", "ofile="])
	except getopt.GetoptError:
		print 'Error: use -h option to see usage!'
		sys.exit(2)

	for opt, arg in opts:
		if opt == '-h':
			print 'Usage: -p <plot type> (currently only 1 type) -x <x axis label> -t <number of x ticks> -y <y axis label> -e <legend location> -b <physical measurements benchmark file> -l <benchmark label> -i <model input file> -a <mode label> -o <output file>'
			sys.exit(2)
		elif opt in ("-p", "--ptype"):
			if plottype != 0:
				print 'Error in option <-p ' + arg + '>: -p flag has already been used! First usage is: <-p ' + str(plottype) + '>'
				sys.exit(2)
			else:
				try:
					plottype = int(arg)
					if plottype != 1 and plottype != 2:
						print 'Error in option <-p ' + arg + '>: <' + arg + '> is out of bounds. Please enter a valid ploy type <1>.'
						sys.exit(2)
				except ValueError, exp:
					print 'Error in option <-p ' + arg + '>: <' + arg + '> is not an integer. Please enter a valid input.'
					sys.exit(2)
		elif opt in ("-x", "--xlabel"):
			if xlabel != '':
				print 'Error in option <-x ' + arg + '>: -x flag has already been used! First usage is: <-x ' + xlabel + '>'
				sys.exit(2)
			else:
				xlabel = arg
		elif opt in ("-t", "--xticks"):
			if xticks != 0:
				print 'Error in option <-t ' + arg + '>: -t flag has already been used! First usage is: <-t ' + str(xticks) + '>'
				sys.exit(2)
			else:
				try:
					xticks = int(arg)
				except ValueError, exp:
					print 'Error in option <-t ' + arg + '>: <' + arg + '> is not an integer. Please enter a valid input.'
					sys.exit(2)
		elif opt in ("-y", "--ylabel"):
			if ylabel != '':
				print 'Error in option <-y ' + arg + '>: -y flag has already been used! First usage is: <-y ' + ylabel + '>'
				sys.exit(2)
			else:
				ylabel = arg
		elif opt in ("-e", "--legendloc"):
			if legendloc != -1:
				print 'Error in option <-e ' + arg + '>: -e flag has already been used! First usage is: <-e ' + str(legendloc) + '>'
				sys.exit(2)
			else:
				try:
					legendloc = int(arg)
				except ValueError, exp:
					print 'Error in option <-e ' + arg + '>: <' + arg + '> is not an integer. Please enter a valid input.'
					sys.exit(2)
		elif opt in ("-b", "--bfile"):
			if bfile != '':
				print 'Error in option <-b ' + arg + '>: -b flag has already been used! First usage is: <-b ' + bfile + '>'
				sys.exit(2)
			else:
				try:
					fileopentest = open(arg, 'r')
					checkreadline = fileopentest.readline()
					if (re.match(r"#Timestamp\tBenchmark\tRun\(#\)\tCPU Frequency\(MHz\)\tCurrent\[A\]\tPower\[W\]\t", checkreadline)) or (re.match(r"#Timestamp\tBenchmark\tRun\(#\)\tCPUFrequency\(MHz\)\tCurrent\[A\]\tPower\[W\]\tEnergy\[J\]\t", checkreadline)):
						fileopentest.close()
						bfile = arg
					else:
						print 'Error in option <-b ' + arg + '>: file <' + arg + '> is not the correct format or is empty. Please enter a valid input file.'
						print 'The results file should start with the following header, followed by the data:'
						print '#Timestamp\tBenchmark\tRun(#)\tCPUFrequency(MHz)\tCurrent[A]\tPower[W]\t*(EVENTS)'
						print 'Or if Energy is computed, the following header:'
						print '#Timestamp\tBenchmark\tRun(#)\tCPUFrequency(MHz)\tCurrent[A]\tPower[W]\tEnergy[J]\t*(EVENTS)'
						print 'NB: This script DOES NOT currently have a way to determine if the data format is correct so please use the proper data collection tool in order to not break the plots.'
						fileopentest.close()
						sys.exit(2)
				except IOError, exp:
					print 'Error in option <-p ' + arg + '>: file <' + arg + '> does not exist. Please enter a valid physical benchmark data file.'
					sys.exit(2)
		elif opt in ("-l", "--blabel"):
			if blabel != '':
				print 'Error in option <-l ' + arg + '>: -l flag has already been used! First usage is: <-l ' + blabel + '>'
				sys.exit(2)
			else:
				blabel = arg
		elif opt in ("-i", "--ifile"):
			try:
				fileopentest = open(arg, 'r')
				checkreadline = fileopentest.readline()
				if arg in inputfile:
					print 'Error in option <-i ' + arg + '>: file <' + arg + '> already selected. Please enter a different input file.'
					fileopentest.close()
					sys.exit(2)
				else:
					if re.match(r"#Sample\[#\]\tPredicted Power\[W\]\tError\[W\]\tAbsolute Percentage Error\[%\]", checkreadline):
						fileopentest.close()
						inputfile.append(arg)
					elif re.match(r"#Sample\[#\]\tPredicted Energy\[J\]\tError\[J\]\tAbsolute Percentage Error\[%\]", checkreadline):
						fileopentest.close()
						inputfile.append(arg)
						energy_flag = 1
					else:
						print 'Error in option <-i ' + arg + '>: file <' + arg + '> is not the correct format or is empty. Please enter a valid input file.'
						print 'The results file should start with the following header, followed by the data:'
						print '#Sample[#]\tPredicted Power[W]\tError[W]\tAbsolute Percentage Error[%]'
						print 'Or if Energy is computed, the following header:'
						print '#Sample[#]\tPredicted Energy[J]\tError[J]\tAbsolute Percentage Error[%]'
						print 'NB: This script DOES NOT currently have a way to determine if the data format is correct so please use the proper data collection tool in order to not break the plots.'
						fileopentest.close()
						sys.exit(2)
			except IOError, exp:
				print 'Error in option <-i ' + arg + '>: file <' + arg + '> does not exist. Please enter a valid input file.'
				sys.exit(2)
		elif opt in ("-a", "--ilabel"):
			if arg in inputlabel:
				print 'Error in option <-a ' + arg + '>: label <' + arg + '> already selected. Please enter a different model label name.'
				sys.exit(2)
			else:
				inputlabel.append(arg)
		elif opt in ("-o", "--ofile"):
			if outputfile != '':
				print 'Error in option <-o ' + arg + '>: -o flag has already been used! First usage is: <-o ' + outputfile + '>'
				sys.exit(2)
			else:
				try:
					fileopentest = open(arg, 'r')
					print 'Filename selected in option <-o ' + arg + '>: file <' + arg + '> already exists. Would you like to overwrite existing file?'
					while True:
						outputfile_userinput = raw_input("Please specify [Y/N]: ")
						if outputfile_userinput == 'Y' or outputfile_userinput == 'y':
							print 'Overwriting specified file.'
							outputfile = arg
							fileopentest.close()
							break
						elif outputfile_userinput == 'N' or outputfile_userinput == 'n':
							print 'Please restart the script with a different output file argument.'
							fileopentest.close()
							quit()
						else:
							print 'Invalid input!'
				except IOError, exp:
					outputfile = arg

	#Checks
	if plottype == 0:
		print 'Please specify plot type with option -p. Use -h for help.'
		sys.exit(2)
	if xlabel == '':
		print 'Please specify x axis label with option -x. Use -h for help.'
		sys.exit(2)
	if xticks == 0:
		print 'Please specify number of x tics with option -t. Use -h for help.'
		sys.exit(2)
	if ylabel == '':
		print 'Please specify y axis label with option -y. Use -h for help.'
		sys.exit(2)
	if bfile != '' and blabel == '':
		print 'Please specify a benchmarks physical measurements file with option -b. Use -h for help.'
		sys.exit(2)
	if inputfile != [] and inputlabel == []:
		print 'Please specify a model input label with option -a. Use -h for help.'
		sys.exit(2)

	if blabel in inputlabel:
		print 'Error in option <-l ' + blabel + '>: benchmark label <' + blabel + '> is present in model labels. Please use a different label for benchmark and models.'
		sys.exit(2)
	if len(inputlabel) != len(inputfile):
		print 'Error in options <-i>: model input file and <-a>: model input label. Please use a label for every model. Number of models is <' + str(len(inputfile)) + '> and number of labels is <' + str(len(inputlabel)) + '>.'
		sys.exit(2)

	if legendloc == -1:
		print 'No legend location specified. Using default value of 0(auto).'
		legendloc = int(0)

	print 'Plot type is ' + str(plottype)
	print 'X Axis label is ' + xlabel
	print 'X tick size is ' + str(xticks)
	print 'Y Axis label is ' + ylabel
	print 'Legend location is ' + str(legendloc)
	print 'Benchmark physical measurements file is ' + bfile
	print 'Benchmark label is ' + blabel
	print 'Model input file is ' + str(inputfile)[1:-1]
	print 'Model label is ' + str(inputlabel)[1:-1]
	print 'Output file is ' + outputfile

	#Data processing
	sample_count=0
	with open(bfile, 'r') as bfile_read:
		for curline in bfile_read:
			# check if the current linestarts with "#"
			if curline.startswith("#"):
				if bfile_header == '':
					bfile_header = curline
			else:
				#sample.append(int(curline.split('\t')[0]))
				sample_count+=1
				sample.append(sample_count)
				if energy_flag == 1:
					phys_value.append(float(curline.split('\t')[6]))
				else:
					phys_value.append(float(curline.split('\t')[5]))

	for num, infile in enumerate(inputfile):
		with open(infile, 'r') as inputfile_read:
			pred_value.append([])
			abs_err.append([])
			rel_err.append([])

			for curline in inputfile_read:
				# check if the current linestarts with "#"
				if curline.startswith("#"):
					if inputfile_header == '':
						inputfile_header = curline
				else:
					pred_value[-1].append(float(curline.split('\t')[1]))
					abs_err[-1].append(float(curline.split('\t')[2]))
					rel_err[-1].append(float(curline.split('\t')[3]))

	#Plotting part of the script
	if plottype == 1:
		#Process aggregated data for plot type 1
		fig, axs = plt.subplots(figsize=(1300/my_dpi, 900/my_dpi))

		color = iter(cm.rainbow(np.linspace(0, 1, len(inputfile)+1)))

		clr = next(color)
		sample_np = np.array(sample)
		data = axs.plot(sample_np-min(sample), phys_value, label=blabel, color=clr)

		for num, infile in enumerate(inputfile):
			clr = next(color)
			axs.plot(sample_np-min(sample)+1, pred_value[num], label=inputlabel[num], color=clr)

		axs.set_xlim(1, len(sample))
		#axs.set_ylim(bottom = 0.024)
		#axs.set_ylim(top = 0.034)

		#Automatically colculate the closest power of 10 to round to (for nice output). Then set the assigned number of xticks 
		#xticks_deccount=0
		#while True:
		#	if (sample_np.size/(xticks-1)) > (10 ** xticks_deccount):
		#		xticks_deccount+=1
		#	else:
		#		xticks_deccount-=2
		#		break

		#sample_np = np.insert((sample_np[int(round(sample_np.size/(xticks-1),-xticks_deccount))-1::int(round(sample_np.size/(xticks-1),-xticks_deccount))]-min(sample)+1), 0, 1)
		
		sample_np = np.insert((sample_np[int((sample_np.size-min(sample)+1)/(xticks-1))::int((sample_np.size-min(sample)+1)/(xticks-1))]-min(sample)+1), 0, 1)
		if sample_np.size == xticks:
			sample_np[sample_np.size-1] = len(sample)
		else:
			sample_np = np.append(sample_np,len(sample))
		
		axs.set_xticks(sample_np)
		axs.set_xticklabels(sample_np, fontsize='small', rotation='45')

		axs.set_xlabel(xlabel, fontweight='bold')
		axs.set_ylabel(ylabel, fontweight='bold')

		axshandles, dud = axs.get_legend_handles_labels()
		#loc is 0 by default otherwise set by user
		axs.legend(handles=axshandles, loc=legendloc)
		#figlegend = pylab.figure(figsize=(2600/my_dpi, 100/my_dpi))
		# copy the handles
		#axshandles = [copy.copy(ha) for ha in axshandles ]
		# set the linewidths to the copies
		#[ha.set_linewidth(10) for ha in axshandles ]
		#figlegend.legend(data, handles=axshandles, loc='center', ncol=4, mode="expand")
		#figlegend.legend(handles=axshandles, bbox_to_anchor=(0., 1.02, 2., .102), loc='lower left', ncol=4, mode="expand", borderaxespad=0.)
		axs.grid()

		fig.tight_layout()
		plt.show()
		#pdfp = PdfPages('legend.pdf')
		#pdfp.savefig(figlegend)
		#pdfp.close()
		#figlegend.show()

		if outputfile != '':
			pdfp = PdfPages(outputfile)
			pdfp.savefig(fig)
			pdfp.close()
			
	#Plotting part of the script
	if plottype == 2:
		#Process aggregated data for plot type 2
		fig, axs = plt.subplots(figsize=(1300/my_dpi, 900/my_dpi))

		color = iter(cm.rainbow(np.linspace(0, 1, len(inputfile)+1)))

		clr = next(color)
		sample_np = np.array(sample)
		#axs.plot(sample_np-min(sample), phys_value, label=blabel, color=clr)
		
		#plt.scatter(sample_np-min(sample), relative_error_vect[:75] * 100.0, c='g', label='relative error bloh')
		
		for num, infile in enumerate(inputfile):
			clr = next(color)
			plt.scatter(sample_np-min(sample)+1, np.divide(np.subtract(phys_value,pred_value[num]),phys_value)*100, label=inputlabel[num], color=clr)
			plt.scatter(sample_np-min(sample)+1, np.full((sample_np-min(sample)+1).shape, np.mean(np.divide(np.subtract(phys_value,pred_value[num]),phys_value)*100)), c='g', marker='_', label='avg relative error')
		
		# First 75 benchmarks are BEEBS, the remaining ones are IRIDA
		#plt.scatter(y[:75], relative_error_vect[:75] * 100.0, c='g', label='relative error bloh')
		#plt.scatter(y[75:], relative_error_vect[75:] * 100.0, c='orange', label='relative error bleh')
		# Print horizontal error marks at each X value
		#plt.scatter(y, np.full(y.shape, mean_relative_error * 100.0), c='g', marker='_', label='avg relative error')
		#plt.scatter(y, np.full(y.shape, mean_abs_relative_error * 100.0), c='r', marker='_', label='+mean(abs(relative error))')
		#plt.scatter(y, np.full(y.shape, -mean_abs_relative_error * 100.0), c='r', marker='_', label='-mean(abs(relative error))')
		#plt.scatter(y, np.full(y.shape, (mean_relative_error + stddev_relative_error) * 100.0), c='b', marker='_', label='relative error + SD')
		#plt.scatter(y, np.full(y.shape, (mean_relative_error - stddev_relative_error) * 100.0), c='b', marker='_', label='relative error - SD')

		#for num, infile in enumerate(inputfile):
			#clr = next(color)
			#axs.plot(sample_np-min(sample)+1, pred_value[num], label=inputlabel[num], color=clr)

		axs.set_xlim(1, len(sample))
		#axs.set_ylim(bottom = 0.024)
		#axs.set_ylim(top = 0.034)

		#Automatically colculate the closest power of 10 to round to (for nice output). Then set the assigned number of xticks 
		#xticks_deccount=0
		#while True:
		#	if (sample_np.size/(xticks-1)) > (10 ** xticks_deccount):
		#		xticks_deccount+=1
		#	else:
		#		xticks_deccount-=2
		#		break

		#sample_np = np.insert((sample_np[int(round(sample_np.size/(xticks-1),-xticks_deccount))-1::int(round(sample_np.size/(xticks-1),-xticks_deccount))]-min(sample)+1), 0, 1)
		
		sample_np = np.insert((sample_np[int((sample_np.size-min(sample)+1)/(xticks-1))::int((sample_np.size-min(sample)+1)/(xticks-1))]-min(sample)+1), 0, 1)
		if sample_np.size == xticks:
			sample_np[sample_np.size-1] = len(sample)
		else:
			sample_np = np.append(sample_np,len(sample))
		
		axs.set_xticks(sample_np)
		axs.set_xticklabels(sample_np, fontsize='small', rotation='45')

		axs.set_xlabel(xlabel, fontweight='bold')
		axs.set_ylabel(ylabel, fontweight='bold')

		axshandles, dud = axs.get_legend_handles_labels()
		#loc is 0 by default otherwise set by user
		#axs.legend(handles=axshandles, loc=legendloc)
		axs.legend(handles=axshandles, bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol=2, mode="expand", borderaxespad=0.)
		axs.grid(True)

		fig.tight_layout()
		plt.show()

		if outputfile != '':
			pdfp = PdfPages(outputfile)
			pdfp.savefig(fig)
			pdfp.close()			
			

#TODO: integrate this plot type (scatter) into script and test			
#from matplotlib import pyplot as plt

# First 75 benchmarks are BEEBS, the remaining ones are IRIDA
#plt.scatter(y[:75], relative_error_vect[:75] * 100.0, c='g', label='relative error BEEBS')
#plt.scatter(y[75:], relative_error_vect[75:] * 100.0, c='orange', label='relative error IRIDA')
# Print horizontal error marks at each X value
#plt.scatter(y, np.full(y.shape, mean_relative_error * 100.0), c='g', marker='_', label='avg relative error')
#plt.scatter(y, np.full(y.shape, mean_abs_relative_error * 100.0), c='r', marker='_', label='+mean(abs(relative error))')
#plt.scatter(y, np.full(y.shape, -mean_abs_relative_error * 100.0), c='r', marker='_', label='-mean(abs(relative error))')
#plt.scatter(y, np.full(y.shape, (mean_relative_error + stddev_relative_error) * 100.0), c='b', marker='_', label='relative error + SD')
#plt.scatter(y, np.full(y.shape, (mean_relative_error - stddev_relative_error) * 100.0), c='b', marker='_', label='relative error - SD')
#plt.xscale('log')
#plt.xlabel("True values")
#plt.ylabel("Relative error [%]")
#plt.legend(loc='upper right')
#plt.grid(True)
#plt.show()
			
			

if __name__ == "__main__":
	main(sys.argv[1:])
