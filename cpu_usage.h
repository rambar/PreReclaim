#include <fstream>

#define PROCSTAT_PATH 		"/proc/stat"
#define PROCPID_PATH		"/proc/%d/stat"

class CPUUsage {
	
private:
	static const int NUM_CPU_TIME_STATES = 10;
	static const int NUM_PROCESS_TIME_STATES = 2;

	/**
	 * user - time spent in user mode.
	 * nice - time spent in user mode with low priority.
	 * system - time spent in system mode.
	 * idle - time spent in the idle task.
	 * iowait - time waiting for I/O to complete.
	 * irq - time servicing hardware interrupts.
	 * softirq - time servicing software interrupts.
	 * steal - time spent in other operating systems when running in a virtualized environment.
	 * guest - time spent running a virtual CPU for guest operating systems.
	 * guest_nice - time spent running a low priority virtual CPU for guest operating systems.
	 */
	enum CPUTimeStates {
	    S_USER = 0,
	    S_NICE,
	    S_SYSTEM,
	    S_IDLE,
	    S_IOWAIT,
	    S_IRQ,
	    S_SOFTIRQ,
	    S_STEAL,
	    S_GUEST,
	    S_GUEST_NICE
	};

	enum ProcTimeStates {
		S_UTIME = 0,
		S_KTIME
	};
	
	int 			pid;
	
	size_t 			prevCpuTime[NUM_CPU_TIME_STATES];
	size_t 			prevProcTimes[NUM_PROCESS_TIME_STATES];

	size_t 			curCpuTime[NUM_CPU_TIME_STATES];
	size_t 			curProcTimes[NUM_PROCESS_TIME_STATES];

	double 			usageActive;
	double 			usageIdle;

	double 			activeTime;
	double 			idleTime;
	double 			totalTime;
	
	double			userActive;
	
	size_t GetIdleTime(size_t *);
	size_t GetActiveTime(size_t *);
	bool ReadCPUStats();
	bool ReadProcStats();
public:
	CPUUsage(const int pid);
	void Tick();
	void CountUsage();
	void GetCPUUsage(double &, double &);
	void GetProcUsage(double &);
};

