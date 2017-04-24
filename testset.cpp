#include <iostream>
#include <thread>

#include "common.h"
#include "launcher.h"
#include "cpu_usage.h"
#include "proc.h"
#include "meminfo.h"

#include "testset.h"

#define KSWAPD_NAME 		"kswapd0"
#define DEFAULT_USAGE_BELOW	18.0
#define DEFAULT_MONITOR_TYPE S_MONITOR_USER_PROC

using namespace std;

void TestSet::PrintSystemUsage(CPUUsage &userUsage, CPUUsage &kswapdUsage, const MemInfo &memInfo) {
	double cpuActive, cpuIdle;
	double userActive, kswapdActive;
	
	userUsage.GetCPUUsage(cpuActive, cpuIdle);
	userUsage.GetProcUsage(userActive);
	kswapdUsage.GetProcUsage(kswapdActive);

	cout.width(2);
	cout << userUsage.GetFrame() << "(";
	cout.precision(1);
	cout << userUsage.GetRunningTime() << "s)";
	
	cout << " - cpu%:(ac/id)";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << cpuActive << "/";
	
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << cpuIdle;

	cout << " - proc:" ;
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << userActive << "%";

	cout << " - kswapd: ";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(4);
	cout.precision(1);
	cout << kswapdActive << "%";

	cout << " - avail Mem: ";
	cout << KBtoMB(memInfo.available) << "MB";

	cout << " - Free Mem: ";
	cout << KBtoMB(memInfo.free) << "MB";
	
	cout << endl;
}

/*
void TestSet::SetMonitor(int target) {
	monitorTarget = target;
}*/

void TestSet::AddTestset(const LaunchType type, const char* const path, const char* const option, const MonitorType monitor, const double usageBelow) {
	Testcase *testcase = new Testcase();
	testcase->type = type;
	testcase->path = path;
	testcase->option = option;
	testcase->monitor = monitor;
	testcase->usageBelow = usageBelow;
	
	listTestset.push_back(testcase);
}

void TestSet::AddForkAndExec(const char* const path, const char* const option) {
	AddTestset(S_LAUNCH_FORK_AND_EXEC, path, option, DEFAULT_MONITOR_TYPE, DEFAULT_USAGE_BELOW);
}

void TestSet::AddForkAndExec(const char* const path, const char* const option, const MonitorType monitor, const double usageBelow) {
	AddTestset(S_LAUNCH_FORK_AND_EXEC, path, option, monitor, usageBelow);
}

void TestSet::AddQuickCommand(const char* const path, const char* const option) {
	AddTestset(S_LAUNCH_QUICK_COMMAND, path, option, DEFAULT_MONITOR_TYPE, DEFAULT_USAGE_BELOW);
}

bool TestSet::StartTest() {
	// kswapd cpu usage measurement
	int kswapdPid = Proc::FindProcessName(KSWAPD_NAME);
	if(kswapdPid == -1) {
		error("kswapd is not enabled\n");
		return 0;
	}

	for (vector<Testcase *>::iterator it = listTestset.begin() ; it != listTestset.end(); ++it) {
		Launcher launcher;
		CPUUsage userUsage;
		CPUUsage kswapdUsage;
		MemInfo memInfo;
		LaunchType type = (*it)->type;
		char* const path = const_cast<char*>((*it)->path);
		char* const option = const_cast<char*>((*it)->option);
		const double usageBelow = (*it)->usageBelow;
		const MonitorType monitor = (*it)->monitor;

		if(type == S_LAUNCH_FORK_AND_EXEC) {			
			int childpid = launcher.forkAndExec(path, option, false);
			if(childpid == -1) {
				error("Launch failed\n");
				return 0;
			}

			cout << "Launching... " << path << " pid:" << childpid << endl;

			userUsage.SetPID(childpid);
			userUsage.SetUsageBelow(usageBelow);
			userUsage.SetUsageMonitor(static_cast<CPUUsage::UsageMonitor>(monitor));
				
			kswapdUsage.SetPID(kswapdPid);
			
			while(true) {
				userUsage.Tick();
				kswapdUsage.Tick();
				
				if(userUsage.IsMeasurable()) {
					memInfo.Read();

					PrintSystemUsage(userUsage, kswapdUsage, memInfo);

					if(userUsage.IsUsageStable()){
						cout << userUsage.GetProcName() << " is stablised at " << userUsage.StablisedAt() << "(s)" << endl;
						break;
					}
				}

				this_thread::sleep_for(chrono::milliseconds(100));
			}
		}
		else if(type == S_LAUNCH_QUICK_COMMAND) {
			int childpid = launcher.forkAndExec(path, option, true);
			if(childpid == -1) {
				error("Launch failed\n");
				return 0;
			}
		}
	}	
}
