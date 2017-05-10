#ifndef _TESTSET_H
#define _TESTSET_H 

#include <vector>
#include "cpu_usage.h"
#include "meminfo.h"

using namespace std;

class TestSet {	
private:
	enum LaunchType {
		S_LAUNCH_FORK_AND_EXEC = 0,
		S_LAUNCH_QUICK_COMMAND,
		S_LAUNCH_AUL_LAUNCH,
		S_LAUNCH_SLEEP,
		S_LAUNCH_PROC_WRITE,
	};
	
	enum MonitorType {
		S_MONITOR_CPU_TOTAL = 0,
		S_MONITOR_USER_PROC,
		S_MONITOR_UNDEFINED,
	};

	struct Constants {
		static const string JSON_LAUNCHTYPE;
		static const string JSON_COMMAND;
		static const string JSON_MONITOR;
		static const string JSON_USAGEBELOW;
		static const string JSON_PROCNAME;

		static const string EFM_PROC_PATH;
		static const string EFM_SIZE_TO_RECLAIM;
		static const string EFM_SIZE_ZERO;

		static const string KSWAPD_NAME;
	};

	class Testcase {
	public: 
		LaunchType	type;
		string 		sparam;
		long		lparam;
		MonitorType monitor;
		double		usageBelow;
		string		procname;
	};

private:
	long monitorPeriod = 100; //default 1000ms
	bool preReclaimEnabled = false;
	vector<Testcase*> listTestset;

public: 
	bool LoadFromFile(string &);
	bool StartTest();

	void SetMonitorPeriod(long milliseconds) { monitorPeriod = milliseconds; }
	void SetPreReclaim(bool preReclaim) { preReclaimEnabled = preReclaim; }
	bool PreReclaimEnabled() { return preReclaimEnabled; }

private:
	void AddAulLaunch(string, const MonitorType, const double, string);
	void AddForkAndExec(string, const MonitorType, const double, string);
	void AddQuickCommand(string);
	void AddSleep(long);
	void AddProcWrite(string, string);
	void AddTestset(const LaunchType, string, long, const MonitorType, const double, string);

	void PrintSystemUsage(CPUUsage &, CPUUsage &, const MemInfo &);
	bool AulLaunch(string);
};

#endif
