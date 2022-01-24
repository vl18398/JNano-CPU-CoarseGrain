/*
	File:	measure_cpu_power.c
	Author:	Matthew Burgess (building on the original work of Varun Anand & Minghui Zhu)

	Top level file containing essential functions to read from CPU PMU counters and launching threads in parallel to launch benchmark programs

	Main functions:

	a57_read_samples		-	reads 3 on-board INA3221 power monitors and sample counter values during benchmark run
	a57_read_samples_pthread	-	creates parallel threads to launch benchmarks
	a57_read_sample_start		-	starts the parallel threads
	a57_read_sample_stop		-	stops the parallel threads launched
	a57_clear_sample_pthread	-	destroys thread at the end of the benchmark run
*/


#define _GNU_SOURCE		// Gives a warning during compile
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "jnano_inst.h"
#include "monitoring.h"
#include "pmu_event_sel.h"
#include "read_counters.h"

typedef unsigned int uint32_t;

FILE *fp_log;
FILE *cl_log;

int start;
pthread_mutex_t mutex;
int stop;
pthread_t thread_ID;
void *exit_status;
extern struct a57_sample *head;

long long int sample_count = 0;

double cpu_power_total = 0;
double in_power_total = 0;

/* read_samples method */
void *a57_read_samples(void *head) {
	unsigned int cpu_power_value;	
	unsigned int in_power_value;

	printf("executing thread\n");

	int start_flag = 0;
	int stop_flag = 0;

	printf("\n\t measure_cpu_pow.c::a57_read_samples() : Initializing counters... \n");

	printf("\n\t measure_cpu_pow.c::a57_read_samples() : Initializing counters...DONE \n");

	printf("\n\t measure_cpu_pow.c::a57_read_samples() : Opening Log File to write...\n");

	fp_log = fopen("power_measurement_log_a57.dat", "w");

	if (!fp_log) {
		printf("cannot open power_measurement_log_a57.dat file \n ");
		return 0;
	}

	int ev_group = 1;		// Select amongst the available group of PMU events - refer pmu_event_sel.h

	printf("\n\t measure_cpu_pow.c::a57_read_samples() : Choosing PMU events for Monitoring...\n");
	choose_events(ev_group);	// Selecting performance counters

	printf("\n\t measure_cpu_pow.c::a57_read_samples() : Reading Counter values prior to BM run\n");

	if (ev_group==1)
	fprintf(fp_log, "#Timestamp\tBenchmark\tRun(#)\tCPU Frequency (MHz)\tCPU Temperature(C)\tCPU Voltage(V)\tCPU Power(W)\tCPU_CYCLES\tL1_DATA_CACHE_ACCESS\tINSTRUCTIONS_RETIRED\tBUS_CYCLES\tL1I_CACHE_ACCESS\tPREDICTABLE_BRANCH_SPECULATIVELY_EXECUTED\n");			// option 1  - ARM RECOMMENDED

	if(ev_group==2)
	fprintf(fp_log, "#Timestamp\tBenchmark\tRun(#)\tCPU Frequency (MHz)\tCPU Temperature(C)\tCPU Voltage(V)\tCPU Power(W)\tL1_DATA_CACHE_ACCESS\tCPU_CYCLES\tL1_INSTR_CACHE_ACCESS\tBUS_CYCLES\tBUS_ACCESS_PERIPHERAL\tBRANCH_SPECULATIVELY_EXECUTED_PROCEDURE_RETURN\n");	// option 2 - Per-frequency model, Jose/Kris

	if(ev_group==3)
	fprintf(fp_log, "#Timestamp\tBenchmark\tRun(#)\tCPU Frequency (MHz)\tCPU Temperature(C)\tCPU Voltage(V)\tCPU Power(W)\tL1_INSTRUCTION_CACHE_ACCESS\tL1_DATA_CACHE_ACCESS\tEXCEPTION_TAKEN\tBRANCH_MISPREDICTION\n");								// option 3 - Rance Rogrigues's model

	if(ev_group==4)
	fprintf(fp_log, "#Timestamp\tBenchmark\tRun(#)\tCPU Frequency (MHz)\tCPU Temperature(C)\tCPU Voltage(V)\tCPU Power(W)\tL1_INSTRUCTION_CACHE_ACCESS\tINSTRUCTION_SPECULATIVELY_EXECUTED\tL2_DATA_CACHE_ACCESS_READ\tBUS_ACCESS\tUNALIGNED_ACCESS\tCPU_CYCLES\n");		// option 4 - Matt Walker's model

	fflush(fp_log);	
	if(feof(fp_log))
	printf("\n\t measure_cpu_pow.c::a57_read_samples() : FAILED to write first line in fp_log FILE... \n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];

	FILE *benchmarks = fopen("./benchmarks","r");

	struct timespec tsample = {0,0};
	float timenow, starttime;

	clock_gettime(CLOCK_MONOTONIC,&tsample);

	starttime = (tsample.tv_sec*1.0e9 + tsample.tv_nsec)/1000000;

	cpu_power_total = 0;
	in_power_total = 0;

	sample_count = 0;

	while(1){
	
		start_flag = 1;
		pthread_mutex_lock(&mutex);

		if(start==0)	start_flag = 0;
		pthread_mutex_unlock(&mutex);
		if(start_flag == 0) continue;

		usleep(100000);		// sleep for 100ms, one sample per 100ms
		
		printf("\t \t measure_cpu_pow.c::a57_read_samples : In WHILE loop. Reading ina3221 meters...\n\n");
		jnano_get_ina3221(VDD_CPU, POWER, &cpu_power_value);

		printf("\t measure_cpu_power::a57_read_samples : Reading CPU power Value: %d \n",cpu_power_value);
		jnano_get_ina3221(VDD_IN, POWER, &in_power_value);

		printf("\t measure_cpu_power::a57_read_samples : Reading Input power Value: %d \n",in_power_value);

		cpu_power_total += cpu_power_value;
		in_power_total += in_power_value;
		printf("\t \t measure_cpu_pow.c::a57_read_samples() Reading ina3221 meters...DONE \n");
		
		sample_count++;
		printf("\t measure_cpu_pow.c::a57_read_samples : In WHILE loop. Sample Count Value: %lld \n\n",sample_count);

		{
			printf("\t measure_cpu_pow.c::a57_read_samples : Reading Counter Values... \n");
			
			disable_counters();	// disables counters prior to reading them (after reading INA meters)
	
			usleep(10000);		/* Adding some delay to prevent counters being 0 all the time during run */			
			
			read_a57_counters('a');	/* read_a57_counters('a') contains init_counters() */
			
			printf("\t measure_cpu_pow.c::a57_read_samples : Reading Counter Values...DONE \n");

			FILE *rate = fopen("/sys/kernel/debug/clock/cpu/rate","r");			// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/kernel/debug/clock/vdd_cpu_mv","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");	// add path to the sysfs file to read the CPU temperature

			size_t elements_freq = fread(a57freq,1,20,rate);
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			rewind(benchmarks);
			size_t elements_bm = fread(a57bench,1,20,benchmarks);

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			clock_gettime(CLOCK_MONOTONIC,&tsample);
			timenow = (tsample.tv_sec*1.0e9+tsample.tv_nsec)/1000000;
			printf("\t measure_cpu_pow.c::a57_read_samples: timenow is: %f \n",timenow);

			fclose(rate);
			fclose(temp);
			fclose(voltage);

			if(a57bench[0]!='\n')
			{
				strtok(a57bench,"\n");

				int cpufreqMHz = atoi(a57freq)/1000000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);
				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);
				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);
				int timesample = (int)(timenow-starttime);	// Duration of BM run

				
				//Activity density values calculated in read_a57_counters('a')
				

				if((cnt0<=0.0)&&(cnt1<=0.0)&&(cnt2<=0.0)&&(cnt3<=0.0)&&(cnt4<=0.0)&&(cnt5<=0.0))
				{
				printf("\t measure_cpu_pow.c::a57_read_samples() : Counter activity Values are 0 or less...\n");
				printf("\t measure_cpu_pow.c::a57_read_samples() : UNABLE to update log file...\n");
				}
				else{
				printf("\t measure_cpu_pow.c::a57_read_samples() : Updating Log file with Counter Activity information...\n");
				fprintf(fp_log, "%d %s 1 %d %d %f %f ", timesample, a57bench, cpufreqMHz , cputempdeg,cpuvoltageV,(cpu_power_total)/1000);
				fflush(fp_log);	
				fprintf(fp_log," %f %f %f %f %f %f \n",cnt0,cnt1,cnt2,cnt3,cnt4,cnt5);		// Updating log file with physical counter stats
				fflush(fp_log);	
				}
				cpu_power_total = 0;
				in_power_total = 0;
			}
		}

		stop_flag = 0;
		pthread_mutex_lock(&mutex);
		if(stop == 1) {	stop_flag = 1; }
		pthread_mutex_unlock(&mutex);

		if(stop_flag==1)
		{
			printf("Breaking sampling thread\n");
			break;
		}
	}		// end of while loop
	fclose(fp_log);
	fclose(benchmarks);	
	return 0;
}

void *a57_read_sample_pthread() {

	pthread_mutex_init(&mutex, NULL);

	pthread_mutex_lock(&mutex);
	stop = 0;
	start = 0;
	pthread_mutex_unlock(&mutex);

	head = (struct a57_sample *)malloc(sizeof (struct a57_sample));
	head->next = NULL;

	cpu_set_t cpu_set2;
	CPU_SET(1, &cpu_set2);
	printf("create thread\n");
	pthread_create(&thread_ID,NULL,a57_read_samples,head);
	sched_setaffinity(thread_ID, sizeof(cpu_set_t), &cpu_set2);

	return head;

}


void a57_read_sample_start() 
{

	pthread_mutex_lock(&mutex);
	sample_count = 0;
	cpu_power_total = 0;
	in_power_total = 0;

	start = 1;
	stop = 0;	
	pthread_mutex_unlock(&mutex);

}


void a57_read_sample_stop() {

	pthread_mutex_lock(&mutex);
	stop = 1;
	start = 0;
	pthread_mutex_unlock(&mutex);
	
	printf("join thread\n");
	pthread_join(thread_ID, &exit_status);
	printf("join thread...DONE\n");
	printf("detach thread...\n");
	pthread_detach(thread_ID);
	printf("detach thread...DONE\n");

}


void a57_clear_sample_pthread(struct a57_sample *head) {

	struct a57_sample *sample = head;
	while (sample!= (struct a57_sample *)NULL) {
		struct a57_sample *next = sample->next;
		free(sample);
		sample = next;
	}
	pthread_mutex_destroy(&mutex);
	printf("\t measure_cpu_pow.c::a57_clear_sample_pthread() : MUTEX DESTROY, clear thread...\n");
}	


void data_retrieval_blackscholes(){

printf("\n\t\t -------------------now in data_retrieval_blackscholes-------------------\n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tblackscholes\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);

			fflush(fp_log);

}


void data_retrieval_dedup(){

printf("\n\t\t -------------------now in data_retrieval_dedup-------------------\n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tdedup\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);

			fflush(fp_log);

}


void data_retrieval_facesim(){

printf("\n\t\t -------------------now in data_retrieval_facesim-------------------\n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tfacesim\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);

			fflush(fp_log);

}


void data_retrieval_ferret(){

printf("\n\t\t -------------------now in data_retrieval_ferret-------------------\n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tferret\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);
		
			fflush(fp_log);

}


void data_retrieval_freqmine(){

printf("\n\t\t -------------------now in data_retrieval_freqmine-------------------\n");




	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tfreqmine\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);
		
			fflush(fp_log);		
	
}


void data_retrieval_fluidanimate(){

printf("\n\t\t -------------------now in data_retrieval_fluidanimate-------------------\n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tfluidanimate\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);

			fflush(fp_log);
		
}


void data_retrieval_streamcluster(){

printf("\n\t\t -------------------now in data_retrieval_streamcluster-------------------\n");

	char a57freq[100];
	char a57temp[100];
	char a57bench[100];
	char cpu_voltage[100];
	char cpu_power[100];
			
			FILE *rate = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r");				// add path to the sysfs file to read the CPU clock
			FILE *voltage = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input","r");		// add path to the sysfs file to read CPU voltage
			FILE *temp = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp","r");					// add path to the sysfs file to read the CPU temperature
			FILE *power = fopen("/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input","r");
			

			size_t elements_freq = fread(a57freq,1,20,rate);
		
			strtok(a57freq,"\n");

			size_t elements_temp = fread(a57temp,1,20,temp);
			strtok(a57temp,"\n");

			size_t elements_volt = fread(cpu_voltage,1,20,voltage);
			strtok(cpu_voltage,"\n");

			size_t elements_power = fread(cpu_power,1,20,power);
			strtok(cpu_power,"\n");
			
			fclose(rate);
			fclose(temp);
			fclose(voltage);
			fclose(power);	
	
				int cpufreqMHz = atoi(a57freq)/1000;		// CPU freq in MHz
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU freq in MHz : %d \n",cpufreqMHz);

				int cputempdeg = atoi(a57temp)/1000;		// CPU temp in degrees
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU temp in degrees : %d \n",cputempdeg);

				float cpuvoltageV = atoi(cpu_voltage)/1000.0;	// CPU voltage in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU Voltage in V : %f \n",cpuvoltageV);

				float cpuvoltageW = atoi(cpu_power)/1000.0;	// CPU power in Volt
				printf("\t measure_cpu_pow.c::a57_read_samples: CPU power in W : %f \n",cpuvoltageW);

			printf("\n\t\t PMU EVENT DATA TRANSFER\n");
	char a[15][15];
int i;

FILE *jpp = fopen("config_read_hw_events.txt","r");
for(i=0; !feof(jpp);i++){
fscanf(jpp,"%s",&a[i]);

}
fclose(jpp);

printf("%s %s %s %s %s %s %s %s\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

			cl_log = fopen("data_store.dat", "a");

			fprintf(cl_log,"1\tstreamcluster\t1\t%d\t%d\t%f\t%f\t%s\t%s\t%s\t%s\t%s\t%s\t\n", cpufreqMHz , cputempdeg,cpuvoltageV,cpuvoltageW,a[2],a[3],a[4],a[5],a[6],a[7]);

			fflush(fp_log);

}


void config_transferPMUdata(){

printf("\n\t\t -------------------now transfering PMU data-------------------\n");

FILE* filell = fopen("config_setup.txt", "r");
 	FILE* fpll = fopen("config_read_hw_events.txt", "w+");
    char line[256]; 
    char str[20];    
    char result; 
    while (fgets(line, sizeof(line), filell)) { 
 
        sscanf(line, "%s ", str); 
        
        printf("%s\t", str); 
	fprintf(fpll,"%s\t",str) ;

   }
 printf("\n");
    fclose(filell);
	fclose(fpll);


}
