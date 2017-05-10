#ifndef _MEMINFO_H
#define _MEMINFO_H 

#define MEMINFO_PATH		"/proc/meminfo"
#define MEMINFO_TOTAL		"MemTotal:"
#define MEMINFO_FREE		"MemFree:"
#define MEMINFO_AVAILABLE	"MemAvailable:"
#define MEMINFO_SWAPTOTAL	"SwapTotal:"
#define MEMINFO_SWAPFREE	"SwapFree:"

class MemInfo {

public:
	unsigned int total;
	unsigned int free;
	unsigned int available;
	unsigned int swaptotal;
	unsigned int swapfree;
	unsigned int swapused;

	bool Read();	
};

#endif
