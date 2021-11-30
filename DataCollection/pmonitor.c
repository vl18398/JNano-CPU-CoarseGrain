#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include "cusparse.h"

#include "utilities.h"
#include "monitoring.h"
#include "jnano_inst.h"
#include "unistd.h"

//extern char log_file_name[1000];
int pc,bm;

int main(void) {


//FILE* fp_v;

//strcpy(log_file_name, "log_power_monitor");
//file_power_profile_create(log_file_name);
//fp_v=fopen(log_file_name, "a");

//	
//printf("Setting GPU freqs\n");

//system("./jetson_clocks.sh --set 76800");


printf("Starting power monitor\n");

int freqs[13] = {76800000,153600000,230400000,307200000,384000000,460800000,537600000,614400000,691200000,768000000,844800000,921600000,998400000};
char cmdbuf[256];



for(pc=0;pc<13;pc++) //multiple performance counters
//for(pc=0;pc<1;pc++) //only one run for idle analysis
{
	//for(bm=0;bm<2;bm++)
	{
		snprintf(cmdbuf,sizeof(cmdbuf),"./jetson_clocks_cool.sh --set 76800000"); 
		system(cmdbuf);
		usleep(10000000000); //wait for same time to setttle temp
		
		power_monitoring_prologue ();

		for(int cfreq=0;cfreq<13;cfreq++)
		{

			printf("Setting GPU freqs to %d\n",freqs[cfreq]);

			snprintf(cmdbuf,sizeof(cmdbuf),"./jetson_clocks.sh --set %d",freqs[cfreq]);
			system(cmdbuf);

			

			//launch benchmarks
			system("./nvidia/run3.sh");

			//system("./nvidia/run2.sh");
			//system("./rodinia/run.sh");
			//usleep(10000000000); //wait for same samples to get idle power

  			printf("Done rodinia\n");
		}
	
		power_monitoring_epilogue();
	}
}

 
printf("Finishing power monitor\n");

printf("Read GPU freqs\n");

system("./jetson_clocks.sh --read");


return 0;	
}







