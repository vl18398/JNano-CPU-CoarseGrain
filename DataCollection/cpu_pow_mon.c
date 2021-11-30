/*
File: cpu_pow_mon.c
Description:	This file is the top file to control the frequencies to be set
		and launches the scripts to start the benchmark run
*/

#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"
#include "monitoring.h"
#include "jnano_inst.h"
#include "unistd.h"

int main(void) {
	printf("\n\t\t -------------------cpu_pow_mon.c-------------------\n");	

	printf("\n\t\t CPU POW MON:: Starting power monitor...\n");

int freqs[39] = {102000,102000,102000, 204000,204000,204000, 307200,307200,307200, 403200,403200,403200, 518400,518400,518400, 614400,614400,614400, 710400,710400,710400, 921600,921600,921600, 1036800,1036800,1036800, 1132800,1132800,1132800, 1224000,1224000,1224000, 1428000,1428000,1428000, 1479000, 1479000, 1479000};	//CPU frequency in kHz

	char cmdbuf[256];

	FILE *fp_log;
	fp_log = fopen("data_store.dat", "w");
if (!fp_log) {
		printf("cannot open power_measurement_log_a57.dat file \n ");	
	}

	printf("\n\t\t -------------------1-------------------\n");
	fprintf(fp_log, "#Timestamp\tBenchmark\tRun(#)\tCPU Frequency (MHz)\tCPU Temperature(C)\tVoltage[V]\tPower[W]\tMEM_ACCESS\tBR_MIS_PRED\tBR_PRED\tL1D_CACHE\tL1D_CACHE_REFILE\tL1I_TLB_REFILE\n");	//

	fflush(fp_log);	
	fclose(fp_log);


printf("\n\t\t -------------------blackscholes-------------------\n");
	for(int cfreq=0;cfreq<39;cfreq++)		// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_blackscholes.sh...\n");
		system("./config_blackscholes.sh"); 	//collect the PMU events
		printf("\tFinished config_blackscholes.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_blackscholes...\n");
		data_retrieval_blackscholes(); 		//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_blackscholes...\n");

		
	}

	
printf("\n\t\t -------------------dedup-------------------\n");
	for(int cfreq=0;cfreq<39;cfreq++)		// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_dedup.sh...\n");
		system("./config_dedup.sh"); 		//collect the PMU events
		printf("\tFinished config_dedup.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_dedup...\n");
		data_retrieval_dedup(); 		//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_dedup...\n");

		
	}


printf("\n\t\t -------------------freqmine-------------------\n");
for(int cfreq=0;cfreq<39;cfreq++)			// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_freqmine.sh...\n");
		system("./config_freqmine.sh"); 	//collect the PMU events
		printf("\tFinished config_freqmine.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_freqmine...\n");
		data_retrieval_freqmine(); 		//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_freqmine...\n");

		
	}


printf("\n\t\t -------------------fruidanimate-------------------\n");
for(int cfreq=0;cfreq<39;cfreq++)			// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_fruidanimate.sh...\n");
		system("./config_fruidanimate.sh"); 	//collect the PMU events
		printf("\tFinished config_fruidanimate.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_fruidanimate...\n");
		data_retrieval_fruidanimate(); 		//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_fruidanimate...\n");

		
	}


printf("\n\t\t -------------------streamcluster-------------------\n");
for(int cfreq=0;cfreq<39;cfreq++)			// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_streamcluster.sh...\n");
		system("./config_streamcluster.sh"); 	//collect the PMU events
		printf("\tFinished config_streamcluster.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_streamcluster...\n");
		data_retrieval_streamcluster();	 	//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_streamcluster...\n");

		
	}


printf("\n\t\t -------------------splash2x.barnes-------------------\n");
for(int cfreq=0;cfreq<39;cfreq++)			// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_splash2x.barnes.sh...\n");
		system("./config_splash2x.barnes.sh"); 	//collect the PMU events
		printf("\tFinished config_splash2x.barnes.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_splash2x.barnes...\n");
		data_retrieval_splash2x_barnes(); 	//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_splash2x.barnes...\n");

		
	}


printf("\n\t\t -------------------splash2x.fft-------------------\n");
for(int cfreq=0;cfreq<39;cfreq++)			// 13 frequencies in all : ALL trials
	{
		printf("\t cpu_pow_mon.c::main() : Setting CPU frequency (kHz) to %d\n",freqs[cfreq]);

		snprintf(cmdbuf,sizeof(cmdbuf),"./cpu_clocks.sh --set %d",freqs[cfreq]);
		system(cmdbuf);
		

		printf("\tLaunching config_splash2x.fft.sh...\n");
		system("./config_splash2x.fft.sh"); 	//collect the PMU events
		printf("\tFinished config_splash2x.fft.sh...\n");

		printf("\tLaunching PMU event transfer...\n");
		config_transferPMUdata(); 		//transfer PMU events data
		printf("\tFinished PMU event transfer...\n");

		printf("\tLaunching data_retrieval_splash2x.fft...\n");
		data_retrieval_splash2x_fft(); 		//collection the data of CPU freq,voltage,power,temperature
		printf("\tFinished data_retrieval_splash2x.fft...\n");

		
	}
	printf("\tStarting Monitoring epilogue...\n");
	power_monitoring_epilogue();
	printf("\tMonitoring epilogue Done...\n");


	printf("\t Finishing power monitor...\n");

	printf("\tRead CPU freqs...\n");
	system("./cpu_clocks.sh --read");

	return 0;	
}
