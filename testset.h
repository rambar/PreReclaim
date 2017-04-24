#ifndef _TESTSET_H
#define _TESTSET_H 

#include <vector>

#include "cpu_usage.h"
#include "meminfo.h"

class TestSet {	
private:
	enum LaunchType {
		S_LAUNCH_FORK_AND_EXEC = 0,
		S_LAUNCH_QUICK_COMMAND,
	};
	
public:	
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
		LaunchType type;
		const char* path;
		const char* option;
	};

	void AddForkAndExec(const char* const, const char* const);
	void AddQuickCommand(const char* const, const char* const);
	bool StartTest();
private:
	std::vector<Testcase*> listTestset;
	void PrintSystemUsage(CPUUsage &userUsage, CPUUsage &kswapdUsage, const MemInfo &memInfo);
};

#endif
