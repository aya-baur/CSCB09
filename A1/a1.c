#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h> 
#include <utmp.h> //USER_PROCESS == ut_type[i]
#include <sys/utsname.h> //system information summary
#include <sys/sysinfo.h>//memory info

void cpuInfo(int samples, int tdelay, int graphics){
    //reference: https://rosettacode.org/wiki/Linux_CPU_utilization#C

    char str[100];
	const char d[2] = " ";
	char* token;
	int i = 0;
	long int sum = 0, idle, lastSum = 0,lastIdle = 0;
	long double cpu_usage;
    int tot_sam = samples;

    printf("\n### CPU Usage Info ###\n");

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
                    //adding all values on line 1 of proc/stat
                    //ex: cpu  5588729 62833 1584022 67561203 758668 0 607505 0 0 0
                    //first token == cpu, the rest are num values
                    sum += atoi(token);
                if(i==3)
                    //at index 3 the value is for idle
                    idle = atoi(token);
                    i++;
                }
        }
        //cpu usage = 100% of cpu - %idle 
        cpu_usage = 100 - (idle-lastIdle)*100.0/(sum-lastSum);

        if(graphics == 1){
            printf("Sample %d CPU usage result: %.2Lf%%\n", tot_sam+1-samples, cpu_usage);
        }else{
            printf("\rTotal CPU usage: %.2Lf%%", cpu_usage);
            fflush(stdout);
        }

        lastIdle = idle;
        lastSum = sum;
        samples--;
        sleep(tdelay);
    }
    if(graphics == 1){
        printf("Total CPU usage: %.2Lf%%", cpu_usage);
    }
    printf("\n");
}

void memoryInfo(int samples, int tdelay, int graphics){
    double lastval = 0;
    struct sysinfo sysInfo;
    printf("\n### Memory ### (Physical Used/Total -- Virtual Used/Total\n");
    for(int i=0; i<samples; i++){
        if(sysinfo(&sysInfo) == -1){
            printf("sysinfo run with error");
            break;
        }
        double totalram = sysInfo.totalram;
        double freeram = sysInfo.freeram;
        double totalswap = sysInfo.totalswap;
        double freeswap = sysInfo.freeswap;
        double usedram = totalram - freeram;
        double usedvirt = usedram + (totalswap - freeswap);
        double totalvirt = totalram + totalswap;
        double convert = 9.31 * pow(10, -10);
        usedram *= convert;
        totalram *= convert;
        usedvirt *= convert;
        totalvirt *= convert;
        printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB", usedram, totalram, usedvirt, totalvirt);
        if(graphics == 1){
            //calculate the relative change
            double change = usedram - lastval;
            if(change == usedram || change == 0){
                printf("    |o 0.00 (%.2f)", usedram);
            }else if(change > 0){
                change *= 100;
                if(change < 1){
                    printf("    |o");
                }else{
                    int changes = change;
                    printf("    |");
                    for (i = 0; i < changes; i ++) {
                        printf("#");
                    }
                    if(change != changes){
                        //if change is not an intger
                        printf("*");
                    }   
                }
                
                change /= 100;
                printf(" %.2f (%.2f)", change, usedram);
            }else if(change < 0){
                change *= -100;
                if(change < 1){
                    printf("    |@");
                }else{
                    int changes = change;
                    printf("    |");
                    for (i = 0; i < changes; i ++) {
                        printf(":");
                    }
                    if(change != changes){
                        //if change is not an integer
                        printf("@");
                    }
                }

                change /= 100;
                printf(" %.2f (%.2f)", change, usedram);
            }
        }
        printf("\n");
        lastval = usedram;
        sleep(tdelay);
    }
}

void userStats(int numUsers){
    //counts the number of users connected
    //have to have this separate function because we are dealing with nonstrings 
    //we need to know the size of array to use
    struct utmp *users;
    users = getutent();
    setutent();
    //just to make sure that if people login/logout at that moment, we can still account for them
    numUsers += 5;
    char usernames[numUsers][UT_NAMESIZE];
    int index = 0;
    while(users != NULL){
        if(users->ut_type == USER_PROCESS){
            if(index == 0){
                memcpy(usernames[index], users->ut_name, UT_NAMESIZE);
                index++;
            }else{
                int exists = 0;
                for(int i=0; i<index; i++){
                     if(memcmp(usernames[i], users->ut_name, 8)==0){
                        exists = 1;
                     }
                }
                if(exists == 0){
                    memcpy(usernames[index], users->ut_name, UT_NAMESIZE);
                    index++;
                }
            }
        }
        users = getutent();
    }
    endutent();
    printf("Number of users connected: %d\n", index);
}

void user(){
    struct utmp *users;
    users = getutent();
    setutent();
    int i=0;
    //need an array of chars to store usernames
    //make sure to not double count users that are connected to several sessions
    int lasti = 0;
    printf("\n### Sessions/Users ###\n");
    while(users != NULL){
        if(users->ut_type == USER_PROCESS){
            printf("%.*s, ", UT_NAMESIZE, users->ut_name);
            printf("%s, ", users->ut_line);
            printf("%.*s\n", UT_HOSTSIZE,  users->ut_host);
            i++;
        }
        users = getutent();
    }
    printf("Number of sessions: %d\n", i);
    setutent();
    userStats(i, users);
    endutent();
}

void utsInfo(){
    struct utsname utsData;
    uname(&utsData);

    printf("\n### System Information ###\n");
    printf("System Name = %s\n", utsData.sysname);
    printf("Machine Name = %s\n", utsData.nodename);
    printf("Version = %s\n", utsData.version);
    printf("Release = %s\n", utsData.release);
    printf("Architecture = %s\n\n", utsData.machine);
}

int main(int argc, char** argv){
    int samples = 10;
    int tdelay = 1;
    int graphics = 1; // 0 for false, 1 for true

    //check that argc >= 2
    if(argc == 2){
        if(strcmp(argv[1], "--system")==0 || strcmp(argv[1], "--s")==0){
            //call function that generates system usage
            memoryInfo(samples,tdelay, graphics);
            cpuInfo(samples, tdelay, graphics);
        }else if(strcmp(argv[1], "--user")==0 || strcmp(argv[1], "--u")==0) {
            //function that generates only user usage
            user();
        }else if(strcmp(argv[1], "--graphics")==0 || strcmp(argv[1], "--g")==0){
            //function that includes graphical output in the cases where graphical outcome is possible
            graphics = 1;
            memoryInfo(samples, tdelay, graphics);
            cpuInfo(samples, tdelay, graphics);
            user();
            utsInfo();
        }
    }else if(argc == 3){
        //see what the args are and decide from there
    }else if(argc == 4){
        // there is 3 arguments here because the first one is the file name
        //see what the args are and decide from there
    }else{
        memoryInfo(samples, tdelay, graphics);
        cpuInfo(samples, tdelay, graphics);
        user();
        utsInfo();
    }

}