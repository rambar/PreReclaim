#define MEMINFO_PATH		"/proc/meminfo"
#define MEMINFO_TOTAL		"MemTotal:"
#define MEMINFO_FREE		"MemFree:"
#define MEMINFO_AVAILABLE	"MemAvailable:"

class MemInfo {

public:
	unsigned int total;
	unsigned int free;
	unsigned int available;

	bool Read();	
};
