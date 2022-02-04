## CSCB09 Assignment 1 Documentation
# Linux Resources Monitor

Author: Ayazhan Bauryzhankyzy<br>
Student #: 1005495386

resources_monitor.c is a program that fetches the following information:
- Total CPU usage
- Physical and Virtual Memory Usage
- Number of users and active sessions connected to the machine
---
---
### How to run the program
>Compile in the terminal using gcc
```console
utorid@iits-b473-01:~/$ gcc resources_monitor.c 
```
Give the executable file a name. For example:
```console
utorid@iits-b473-01:~/$ gcc resources_monitor.c -o output
```
To see the output:
```console
utorid@iits-b473-01:~/$ ./output
```
---
---
### Flags
Following flags can be used when outputting the results<br><br>
Program will return only user information:
```console
--user [or] -u
```
Program will return system information:
```console
--system [or] -s
```
Program will include graphical representation of system information:
```console
--graphics [or] -g
```
To specify number of samples to take when compiling use:
```console
--samples=N
```
where N = integer specifying number of samples

To indicatre how frequently to sample in seconds:
```console
--tdelay=T
```
where T = integer indicating sampling frequency in seconds

> samples and tdelay are also positional arguments in the respective order: 
```console
5 2
```
>>program will take 5 samples every 2 seconds
---
---
## Functions Overview
```c
void cpuInfo(int samples, int tdelay, int graphics){
    //implementation
}
```
Function takes 3 parameters for the number of samples, tdelay seconds, and boolean graphics

Implementation is based off of the code provided here: https://rosettacode.org/wiki/Linux_CPU_utilization#C 
<br>
This function gets CPU information from /proc/stat, where the first line contains information that it uses to calculate total cpu usageM<br>
CPU usage = 100% - idle%<br>

if graphics flag was used, function prints every sample value with percentage representation using | bars

---
```c
void memoryInfo(int samples, int tdelay, int graphics){
    //implementation
}
```
Function takes 3 parameters for the number of samples, tdelay seconds, and boolean graphics
```c
#include <sys/sysinfo.h>
```
Sysinfo library is used to access memory information
- Used memory = totalram - freedram
- Virtual memory = totalram + totalswap
- Used virtual memory = usedram + (totalswap - freeswap)
<br>
> If graphics flag was used<br>

>> Change in usedram is calculated and graphically represented in a following way:

>>> If the change was positive and significant *(100 * change > 1), we print (int)(100 * change) of #s* <br> followed by * if 100 * change is not an integer
>>> If the change was negative and significant, we print *(100 * change) of @s*
---

```c
void userStats(int numUsers, struct utmp* users){
    //implementation
}
```

Function takes 2 parameters
- numUsers = number of sessions connected to the machine
- users = struct utmp* that provides us the information about connected users
<br>
These parameters are passed to this function from user()
<br>
Purpose of this function is to count the number of users connected to the machine, it exists seperately from the session counter because we are dealing with nonstrings. Thus, it is required to have numUsers to be able to properly count the number of users
---

```c
void user(int samples, int tdelay){
    //implementation
}
```

Function takes 2 parameters
- samples
- tdelay
<br>
It gets user information from utmp.h library sample number of times with tdelay seconds of delay
```c
#include <utmp.h>
```
---
```c
void utsInfo(){
    //implementation
}
```
Function retrieves and displays system information from utsname.h library
```c
#include <sys/utsname.h>
```
---
```c
void runFunctions(int sys, int u, int graphics, int samples, int tdelay){
    //implementation
}
```
This is a helper function that manages which functions to call based on the user flags provided as parameters

---
```c
int main(int argc, char** argv){
    //implementation
}
```
Retrieves user arguments provided and passes the flags to runFunctions() to call proper functions
