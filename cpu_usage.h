#ifndef _CPU_USAGE_H
#define _CPU_USAGE_H 

#include <fstream>
#include <chrono>

#define PROCSTAT_PATH 		"/proc/stat"
#define PROCPID_PATH		"/proc/%d/stat"

using namespace std;

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
		S_USAGE_ABOVE = 0,
		S_USAGE_BELOW,
		S_USAGE_STABLIZED
	};

public:
	enum UsageMonitor {
		S_USAGE_CPU_TOTAL = 0,
		S_USAGE_USER_PROC = 1,
	};

private:	
	int 			pid = -1;
	string			pname;
	
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
	chrono::system_clock::time_point	tickStart;
	chrono::system_clock::time_point	tickLast;

	UsageMonitor	usageMonitor;
	double 			usageBelow = -1.0;
	double 			secToStay = 3.0;
	UsageStable 	isUsageStable = S_USAGE_ABOVE;
	chrono::system_clock::time_point	tickUsageBelow;
	
	size_t GetIdleTime(size_t *);
	size_t GetActiveTime(size_t *);
	bool ReadCPUStats();
	bool ReadProcStats();
	void CountUsage();
	double TimeBetween(chrono::system_clock::time_point a, chrono::system_clock::time_point b);
	void CheckUsageStable(const double);

public:	
	CPUUsage() {}
	CPUUsage(const int pid);
	
	void Tick();
	void GetCPUUsage(double &, double &);
	void GetProcUsage(double &);
	void SetProcName(string &name);
	string& GetProcName() { return pname; };
	//void GetProcName(string &);
	unsigned int GetFrame() { return frame; }
	void SetUsageBelow(const double usageBelow) { this->usageBelow = usageBelow; }
	void SetUsageMonitor(const UsageMonitor usageMonitor) { this->usageMonitor = usageMonitor;} 
	unsigned int GetPID() { return pid; }
	void SetPID(const unsigned int pid);
	double GetRunningTime() { return TimeBetween(tickStart, tickLast); }
	bool IsMeasurable() { return measurable; }

	double StablisedAt() { return TimeBetween(tickStart, tickUsageBelow); }
	bool IsUsageStable();
};

#endif
