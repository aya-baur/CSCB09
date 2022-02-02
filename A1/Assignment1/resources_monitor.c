#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <utmp.h>
#include <err.h>

void utsInfo(){
    struct utsname utsData;
    uname(&utsData);

    printf("\n### System Information ###\n");
    printf("System Name = %s\n", utsData.sysname);
    printf("Machine Name = %s\n", utsData.nodename);
    printf("Version = %s\n", utsData.version);
    printf("Release = %s\n", utsData.release);
    printf("Architecture = %s\n", utsData.machine);
}

void systemMemory(int samples, int tdelay){
    struct sysinfo sysInfo;
    
    printf("\n### Memory ### (Phys. Used/Tot -- Virtual Used/Tot)\n");
    for(int i=0; i < samples; i++){
        if(sysinfo(&sysInfo)<0){
            printf("sysinfo run with error");
            break;
        }
        // phys. used/total = (total ram - free ram) / total ram
        double phys_total = sysInfo.totalram;
        double phys_used = phys_total - sysInfo.freeram;
        // virtual used/total = [(total ram - free ram) + (total swap - free swap)] / total ram + total swap
        double virt_total = sysInfo.totalram + sysInfo.totalswap;
        double virt_used = phys_used + (sysInfo.totalswap - sysInfo.freeswap);
        double convert = 9.31 * pow(10,-10);
        phys_total *= convert;
        phys_used *= convert;
        virt_total *= convert;
        virt_used *= convert;
        printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", 
                phys_used, phys_total, virt_used, virt_total);
        sleep(tdelay);
    }
    
}

void cpuInfo(int samples, int tdelay, int graphics){

    //reference: https://rosettacode.org/wiki/Linux_CPU_utilization 
    
    char str[100];
	const char d[2] = " ";
	char* token;
	int i = 0;
	long int sum = 0, idle, lastSum = 0,lastIdle = 0;
	long double usageOfCPU;

    printf("\n### CPU Usage Information ###\n");

	while(samples>0){
		FILE* fp = fopen("/proc/stat","r");
	    i = 0;
		fgets(str,100,fp);
		fclose(fp);
		token = strtok(str,d);
	    sum = 0;
		while(token!=NULL){
			token = strtok(NULL,d);
			if(token!=NULL){
				sum += atoi(token);

			if(i==3)
				idle = atoi(token);

			i++;
		    }
	    }
	    usageOfCPU = 100 - (idle-lastIdle)*100.0/(sum-lastSum);

        printf("\rTotal cpu use: %.2Lf%%", usageOfCPU);

        if(graphics == 1){
            printf("Busy for : %.2Lf%% of the time.\n", usageOfCPU);
        }
	    lastIdle = idle;
	    lastSum = sum;
	    samples--;
	    sleep(tdelay);

        fflush(stdout);
	}	
    printf("\n");
	

}

void user(){
    //report how many users are connected in a given time
    int numUsers = 0;
    //report how many sessions each user is connected to
    printf("\n### Sessions/users ###\n");
    struct utmp *utm;
    utm = getutent();
    while(getutent() != NULL){
        if(utm->ut_type == USER_PROCESS){
            printf("%s, %s (%s)\n", utm->ut_user, utm->ut_line, utm->ut_host);
            numUsers++;
        }
    }
    printf("There are %d users connected at this time\n", numUsers);
}


int main(int argc, char** argv){
    //argc is to know how many strings to expect in array
    //argv[1] contains the first command line argument
    int samples = 10;
    int tdelay = 1;
    int graphics = 1; //0 for false and 1 for true

    // if(argC!=3)
	// 	printf("Usage : %s <number of times /proc/stat should be polled followed by delay in seconds.>",argV[0]);
 

    if(argc > 1){
        if(strcmp(argv[1], "--system")==0 || strcmp(argv[1], "--s")==0){
            //call function that generates system usage only
            //output system usage
            utsInfo();
            systemMemory(samples, tdelay);
        }else if(strcmp(argv[1], "--user")==0 || strcmp(argv[1], "--u")==0){
            //function that generates only user usage
            user();
        }else if(strcmp(argv[1], "--graphics")==0 || strcmp(argv[1], "--g")==0){
            //function that includes graphical output in the cases where graphical outcome is possible
            //graphics();
        }else if(strcmp(argv[1], "t")==0){
            cpuInfo(samples,tdelay, graphics);
        }else if(strncmp(argv[1], "--samples", 9)==0){
            //--samples=N  
            // N = number of times the stats are going to be collected
            // results = average of N stats
            // default N=10

            //the stats include user usage and system usage
                //aka what the first two flags return

        }else if(strncmp(argv[1], "--tdelay", 8)==0){
            // --tdelay=T 
            //how frequently to sample in seconds
            // default T=1 second
        }else{
            printf("Invalid Argument");
        }
    }else{
        //if No Arguments are provided, run samples stats
        utsInfo();
        systemMemory(samples, tdelay);
        cpuInfo(samples, tdelay, graphics);
        user();
    }
    

    return 0;
}