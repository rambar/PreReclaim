#ifndef _MEMINFO_H
#define _MEMINFO_H 

#define MEMINFO_PATH			"/proc/meminfo"
#define MEMINFO_TOTAL			"MemTotal:"
#define MEMINFO_FREE			"MemFree:"
#define MEMINFO_AVAILABLE		"MemAvailable:"
#define MEMINFO_SWAPTOTAL		"SwapTotal:"
#define MEMINFO_SWAPFREE		"SwapFree:"
#define MEMINFO_SWAPFREE		"SwapFree:"
#define MEMINFO_ACTIVEFILE		"Active(file):"
#define MEMINFO_INACTIVEFILE	"Inactive(file):"

class MemInfo {

public:
	unsigned int total;
	unsigned int free;
	unsigned int available;
	unsigned int swaptotal;
	unsigned int swapfree;
	unsigned int swapused;
	unsigned int pagecache;

	bool Read();	
};

#endif
