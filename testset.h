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
		S_LAUNCH_AUL_LAUNCH,
		S_LAUNCH_SLEEP,
		S_LAUNCH_PRE_RECLAIM,
		S_LAUNCH_WAIT_STABLISED,
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
		static const string JSON_WAITSTABLIZED;
		static const string JSON_PROCNAME;

		static const string EFM_PROC_PATH;
		static const string EFM_SIZE_TO_RECLAIM;
		static const string ZERO;

		static const string KSWAPD_NAME;
		static const string RESOURCED_NAME;
	};

	class Testcase {
	public: 
		LaunchType	type;
		string 		sparam;
		long		lparam;
		MonitorType monitor;
		double		usageBelow;
		string		procname;
		bool 		waitStablized;
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
	void AddAulLaunch(string appid, const MonitorType monitor, const double usageBelow, string procname, bool waitStablized);
	void AddForkAndExec(string command, const MonitorType monitor, const double usageBelow, string procname, bool waitStablized);
	void AddSleep(long);
	void AddPreReclaim(string);
	void AddWaitStablised(const double usageBelow);
	
	void AddTestset(const LaunchType type, string sparam, long lparam, const MonitorType monitor, const double usageBelow, string procname, bool waitStablized);

	void PrintSystemUsageHeader();
	void PrintSystemUsage(CPUUsage &, CPUUsage &, CPUUsage &, const MemInfo &);
	bool AulLaunch(string);

	void FillProcname(string &procname, string &sparam);
};

#endif
