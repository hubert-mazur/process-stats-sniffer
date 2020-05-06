# <p style="text-align:center;color:blue">process-stats-sniffer</p>

Process statistics sniffer written in C for UNIX platform

### Prerequisites
- gcc compiler with C11 standard
- ncurses library installed
- UNIX OS :) (tested on Ubuntu 20.04 Focal Fossa)
- root privilleges

### Description
- Process-stats-sniffer is UNIX* platform driven program written in C
- Reads /proc/ directory for obtaining information about running processes
- Process statistics like:
    - Process ID (PID)
    - Parent Process ID (PPID)
    - Process state (status) eg. running
    - Filename of executable (command)
    - Amount of time in user mode (utime)
    - Amount of time in kernel mode (stime)
    - Virtual memory size (vsize)
    - Resident Set Size (rss)
    - Program size (size)
    - Resident shared pages (shared)
    - Characters read - CURRENTLY NOT LISTED
    - Characters written - CURRENTLY NOT LISTED
    - Bytes read - CURRENTLY NOT LISTED
    - Bytes written - CURRENTLY NOT LISTED
    
For interactive display ncurses library is used.


### Compilation & Run
Clone repository and go to destination directory, eg.
``` git clone https://github.com/hubert-mazur/process-stats-sniffer && cd process-stats-sniffer ```

For compiling, you can use simple:
``` gcc process.c main.c -lncurses -lpthread -o process-stats-sniffer ```

Change owner of compilation output file
``` sudo chown root:root process-stats-sniffer```

Change permissions
``` sudo chmod +s process-stats-sniffer ```

Run
``` ./process-stats-sniffer ```

### Command line arguments

While starting app you may give few parameters in order to customize output:
- ```-h lists processes statistics with human readable values ```
- ```--sortby=... sorting processes by given parameter, possible sortings: ```
    - ```pid - sort by PID ascending (default),```
    - ```ppid - sort by PPID ascending ```
    - ```vsize - sort by virtual memory size descending ```
    - ```rss - sort by resident set size descending ```
    - ```utime - sort by time spent in user mode descending```
    - ```stime - sort by time spent in kernel mode descending```
    - ```size - sort by program size descending```
    - ```shared - sort by shared memory descending```
- ```--watch refreshes with given currency in seconds - CURRENTLY DOES NOT TAKE ANY EFFECT ```
- ```--rsslim=... - set limit for rss, process exceeding will be listed on red```
- ```--vsize= - set limit for virtual size```
- ```--size - set limit for program size```
- ```--shared - set limit for shared memory```
- ```--stime - set limit for time spent in kernel mode```
- ```--utime - set limit for time spent in user mode```

When giving limit it is possible to use prefixes:
- k - kilo (10^3)
- M - Mega (10^6)
- G - Giga (10^9)

### Examples of usage

```./process-stats-sniffer --rsslim=10M --sortby=rss```

```./process-stats-sniffer -h --sortby=utime```

```./process-stats-sniffer --size=100 --sortby=ppid```

### Window
- You can exit program with 'q' button
- You can move up and down in list with 'w' and 's' buttons