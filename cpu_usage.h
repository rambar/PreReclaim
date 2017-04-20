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

	enum UsageStable {
		S_ABOVE = 0,
		S_BELOW,
		S_STABLIZED
	};
	
	int 			pid;
	
	size_t 			prevCpuTime[NUM_CPU_TIME_STATES];
	size_t 			prevProcTimes[NUM_PROCESS_TIME_STATES];
	
	size_t 			curCpuTime[NUM_CPU_TIME_STATES];
	size_t 			curProcTimes[NUM_PROCESS_TIME_STATES];

	double 			cpuActive; //CPU active 
	double 			cpuIdle;   //CPU idle

	double 			activeTime;
	double 			idleTime;
	double 			totalTime;
	
	double			userActive; //User process active
	unsigned int	frame = 0;

	bool			firstTick = true;
	bool			measurable = false;
	std::chrono::system_clock::time_point	tickStart;
	std::chrono::system_clock::time_point	tickLast;

	double usageBelow = 18.0;
	double secToStay = 3.0;
	UsageStable isUsageStable = S_ABOVE;
	std::chrono::system_clock::time_point	tickUsageBelow;
	
	size_t GetIdleTime(size_t *);
	size_t GetActiveTime(size_t *);
	bool ReadCPUStats();
	bool ReadProcStats();
	void CountUsage();
	double TimeBetween(std::chrono::system_clock::time_point a, std::chrono::system_clock::time_point b);
	void CheckUsageStable();

public:	
	CPUUsage(const int pid);
	
	void Tick();
	void GetCPUUsage(double &, double &);
	void GetProcUsage(double &);
	unsigned int GetFrame() { return frame; }
	unsigned int GetPID() { return pid; }
	double GetRunningTime() { return TimeBetween(tickStart, tickLast); }
	bool IsMeasurable() { return measurable; }
	
	bool IsUsageStable() { return (isUsageStable == S_STABLIZED); }
	double StablisedAt() { return TimeBetween(tickStart, tickUsageBelow); }
};

