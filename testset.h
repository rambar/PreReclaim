#ifndef _TESTSET_H
#define _TESTSET_H 

#include <vector>

#include "cpu_usage.h"
#include "meminfo.h"

#define TESTCASE_SPARAM_MAX 256

class TestSet {	
private:
	long monitorPeriod = 100; //default 1000ms
	bool preReclaimEnabled = false;
	
public:	
	enum LaunchType {
		S_LAUNCH_FORK_AND_EXEC = 0,
		S_LAUNCH_QUICK_COMMAND,
		S_LAUNCH_AUL_LAUNCH,
		S_LAUNCH_SLEEP,
		S_LAUNCH_PROC_WRITE,
	};
	
	enum MonitorType {
		S_MONITOR_CPU_TOTAL = 0,
		S_MONITOR_USER_PROC = 1,	
	};
	
	/*enum MonitorTarget {
		S_MONITOR_CPU		= 0x0001,
		S_MONITOR_MEMORY	= 0x0002,
		S_MONITOR_KSWAPD	= 0x0004,	
	};
	void SetMonitor(int target);
	int monitorTarget;
	*/

	class Testcase {
	public: 
		LaunchType 	type;
		std::string sparam;
		long		lparam;
		MonitorType monitor;
		double 		usageBelow;
	};

	void AddAulLaunch(std::string );
	void AddAulLaunch(std::string , const MonitorType, const double);
	void AddForkAndExec(std::string);
	void AddForkAndExec(std::string , const MonitorType, const double);
	void AddQuickCommand(std::string );
	void AddSleep(long);
	void AddProcWrite(std::string, std::string);
		
	bool StartTest();

	void SetMonitorPeriod(long milliseconds) { monitorPeriod = milliseconds; }
	void SetPreReclaim(bool preReclaim) { preReclaimEnabled = preReclaim; }
	bool PreReclaimEnabled() { return preReclaimEnabled; }
	
private:
	std::vector<Testcase*> listTestset;
	void PrintSystemUsage(CPUUsage &, CPUUsage &, const MemInfo &);
	void AddTestset(const LaunchType, std::string , long, const MonitorType, const double);
	bool AulLaunch(std::string);
};

#endif
