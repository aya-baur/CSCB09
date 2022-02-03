#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h> 
#include <utmp.h> 
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
	    		sum += atoi(token);
	    	if(i==3)
	    		idle = atoi(token);
	    		i++;
	    	}
	    }
        cpu_usage = 100 - (idle-lastIdle)*100.0/(sum-lastSum);

        if(graphics == 1){
            int bars = cpu_usage;
            for(int j=0; j<cpu_usage+2; j++){
                printf("|");
            }
            printf(" %.2Lf%%\n", cpu_usage);
        }else{
            printf("\rTotal CPU use = %.2Lf%%", cpu_usage);
            fflush(stdout);
        }

	    lastIdle = idle;
        lastSum = sum;
	    samples--;
	    sleep(tdelay);
    }
    if(graphics == 1){
        printf("\nTotal CPU use = %.2Lf%%", cpu_usage);
    }
    printf("\n");
}

void memoryInfo(int samples, int tdelay, int graphics){
    double lastval = 0;
    struct sysinfo sysInfo;
    printf("\n### Memory ### (Physical Used/Total -- Virtual Used/Total)\n");
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
                    for (int k = 0; k < changes; k++) {
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
                    for (int k = 0; k < changes; k++) {
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

void userStats(int numUsers, struct utmp* users){
    users = getutent();
    setutent();
    //just to make sure that if other people login/logout, we can still account for them
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

void user(int samples, int tdelay){
    printf("\n### Sessions/Users ###\n");
    for(int s = 0; s < samples; s++){
        struct utmp *users;
        users = getutent();
        setutent();
        int i=0;
        //need an array of chars to store usernames
        //make sure to not double count users that are connected to several sessions
        while(users != NULL){
            if(users->ut_type == USER_PROCESS){
                if(s == samples-1){
                    printf("%.*s, ", UT_NAMESIZE, users->ut_name);
                    printf("%s, ", users->ut_line);
                    printf("%.*s\n", UT_HOSTSIZE,  users->ut_host);
                }
                i++;
            }
            users = getutent();
        }
        printf("\nNumber of sessions: %d\n", i);
        setutent();
        userStats(i, users);
        endutent();
        if(s != samples-1){
            // printf("\033[%dA", i+3);
            printf("\033[3A");
            fflush(stdout);
        }
        sleep(tdelay);
    }
}

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

void runFunctions(int sys, int u, int graphics, int samples, int tdelay){
    printf("\nNumber of samples: %d -- every %d seconds\n", samples, tdelay);
    printf("---------------------------------------------------------------");

    if(sys == 1){
        memoryInfo(samples,tdelay, graphics);
        printf("---------------------------------------------------------------");
        cpuInfo(samples, tdelay, graphics);
        printf("---------------------------------------------------------------");
    }
    if(u == 1){
        user(samples, tdelay);
        printf("---------------------------------------------------------------");
    }
    if(sys == 0 && u == 0){
        memoryInfo(samples, tdelay, graphics);
        printf("---------------------------------------------------------------");
        cpuInfo(samples, tdelay, graphics);
        printf("---------------------------------------------------------------");
        user(samples, tdelay);
        printf("---------------------------------------------------------------");
        utsInfo();
        printf("---------------------------------------------------------------");
    }
    printf("\n");
}

int main(int argc, char** argv){
    int samples = 10;
    int tdelay = 1;

    //getting all flags
    // 0 for false and 1 for true
    int sys = 0;
    int user = 0;
    int graphics = 0;

    int secondDigit = 0;

    if(argc>1){
        for(int j=1; j<argc; j++){
            if(strcmp(argv[j], "--system")==0 || strcmp(argv[j], "-s")==0){
                sys = 1;
            }else if(strcmp(argv[j], "--user")==0 || strcmp(argv[j], "-u")==0) {
                user = 1;
            }else if(strcmp(argv[j], "--graphics")==0 || strcmp(argv[j], "-g")==0){
                graphics = 1;
            }else if(strncmp(argv[j], "--samples=", 10)==0){
                char* ptr;
                ptr = &argv[j][10];
                samples = atoi(ptr);
            }else if(strncmp(argv[j], "--tdelay=", 9)==0){
                char* ptr;
                ptr = &argv[j][9];
                tdelay = atoi(ptr);

            }else if(isdigit(*argv[j]) != 0){
                if(secondDigit == 0){
                    samples = atoi(argv[j]);
                    secondDigit = 1;
                }else{
                    tdelay = atoi(argv[j]);
                }  
            }else{
                printf("Invalid Argument: %s\n", argv[j]);
            }
        }
    }

    runFunctions(sys, user, graphics, samples, tdelay);
    return 0;
}
