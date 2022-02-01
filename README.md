# JNano-CPU-CoarseGrain
# Power Modelling of ARM Cortex-A57 CPU on NVIDIA Jetson Nano

*******************************************************************************
# Step 1: Data Collection (Requires NVIDIA Jetson Nano)
1)  Go to DataCollection/
2)  In each of the config_*benchmarkname* files, change the register address values to correspond to the target PMU group
3)  In line 30 of cpu_pow_mon.c, change the PMU event names to correlate to the desired PMU register addresses
4)  Run the makefile to compile the files: 
            make -f Makefile_cpu cpu_pow
4)  Run executable to start the benchmark run and collect data: 
            sudo ./pmon_cpu
5)  Raw data file is dumped: 
            data_store.dat
6)  Repeat this process until all 14 PMU event groups capture all 84 PMU events (14*6)
*******************************************************************************

# Step 2: Post Processing (Offline)
1)  In order to concatenate all of the PMU events into one singular .dat file, each of the 14 data files must be combined

**USING MS EXCEL**

2)  Split each .dat file for each PMU group into files for each benchmark
3)  Concatenate all of the PMU events for a target benchmark into one file, and take the average of all the physical sensor recordings
4)  Concatenate all separate benchmark files to create a master file, power_measurement_merged_global.txt
***********************************************************************************

# Step 3: Power Modelling and Validation
1)  Go to ModellingAndValidation/
2)  Ensure the following files are present in the directory:
	   benchmark_split.txt
	   build_model.m,
	   load_build_model.m,
	   octave_makemodel.sh (Make it executable using chmod u+x
    octave_makemodel.sh), and
	   power_measurement_merged_global.txt
3)  Generate and Validate the power model by launching the command: 
            make make_cpu_model
4)  The model details can be output into an output log using the -s option
5)  Use ./octave_makemodel.sh -h to open the help menu

#################################################################################

CREDITS:
1)  Dr Jose Nunez Yanez, Department of Electrical and Electronic Engineering, University of Bristol
2)  Dr Kris Nikov, Department of Computer Science, University of Bristol
3)  Varun Anand, MSc Microelectronics Student, University of Bristol

This project work serves as an extension to the work carried out on the NVIDIA Jetson boards by Dr Jose Nunez-Yanez
and Dr Kris Nikov with their published work for the Maxwell GPU on Jetson TX1.
It also leads on from the work completed by MSc student Varun Anand, for the ARM Cortex-A57 CPU.
*********************************************************************************

REFERENCES:
1)  GPU Power Model: https://github.com/kranik/ARMPM_BUILDMODEL/tree/GPU_tx1
2)  Original Modelling scripts adopted from Dr Kris Nikov's Doctoral Thesis
    project: https://github.com/kranik/ARMPM_BUILDMODEL/tree/master
3)  Kris Nikov. Robust Energy and Power Predictor Selection. url: 
    https://github.com/TSL-UOB/TP-REPPS
4)  Kris Nikov and Jose Nunez-Yanez. "Intra and Inter-Core Power Modelling
    for Single-ISA Heterogeneous Processors". In: International Journal
    of Embedded Systems (2020)

#################################################################################
