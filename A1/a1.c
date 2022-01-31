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
            printf("Busy for : %.2Lf%% of the time.\n", cpu_usage);
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
        printf("Total CPU usage: %.2Lf%%\n", cpu_usage);
    }
    printf("\n");
}

void memoryInfo(int samples, int tdelay){
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
        printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", usedram, totalram, usedvirt, totalvirt);
        sleep(tdelay);
    }
}

void userStats(int numUsers){
    struct utmp *users;
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

int main(int argc, char** argv){
    int samples = 10;
    int tdelay = 1;
    int graphics = 1; // 0 for false, 1 for true

    //check that argc >= 2
    if(argc == 2){
        if(strcmp(argv[1], "--system")==0){
            //call function that generates system usage
            memoryInfo(samples,tdelay);
            cpuInfo(samples, tdelay, graphics);
        }else if(strcmp(argv[1], "--user")==0){
            //function that generates only user usage
            user();
        }else if(strcmp(argv[1], "--graphics")==0){
            //function that includes graphical output in the cases where graphical outcome is possible
        }
    }else if(argc == 3){
        //see what the args are and decide from there
    }else if(argc == 4){
        // there is 3 arguments here because the first one is the file name
        //see what the args are and decide from there
    }else{
        memoryInfo(samples, tdelay);
        user();
        cpuInfo(samples, tdelay, graphics);
    }

}