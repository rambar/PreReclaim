#include <iostream>
#include <thread>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <dirent.h>

#include "common.h"
#include "launcher.h"
#include "cpu_usage.h"
#include "proc.h"
#include "meminfo.h"
#include "testset.h"
#include "streamline_annotate.h"

#if defined(TIZEN)
#include "aul.h"
#include "bundle.h"
#endif

#define KSWAPD_NAME 			"kswapd0"
#define DEFAULT_USAGE_BELOW		18.0
#define DEFAULT_MONITOR_TYPE 	S_MONITOR_USER_PROC

using namespace std;

void TestSet::PrintSystemUsage(CPUUsage &userUsage, CPUUsage &kswapdUsage, const MemInfo &memInfo) {
	double cpuActive, cpuIdle;
	double userActive, kswapdActive;
	
	userUsage.GetCPUUsage(cpuActive, cpuIdle);
	userUsage.GetProcUsage(userActive);
	kswapdUsage.GetProcUsage(kswapdActive);

	cout.width(4);
	cout.precision(1);
	cout << userUsage.GetRunningTime() << "s";
	
	cout << " - cpu%:(ac/id)";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(3);
	cout.precision(0);
	cout << cpuActive << "/";
	
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(3);
	cout.precision(0);
	cout << cpuIdle;

	cout << " - proc:" ;
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(3);
	cout.precision(0);
	cout << userActive << "%";

	cout << " - kswapd:";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(2);
	cout.precision(0);
	cout << kswapdActive << "%";

	cout << " - mem(MB):(av/fr) ";
	cout << KBtoMB(memInfo.available) << "/";
	cout << KBtoMB(memInfo.free);
	
	cout << endl;
}

void TestSet::AddTestset(const LaunchType type, string sparam, long lparam, const MonitorType monitor, const double usageBelow) {
	Testcase *testcase = new Testcase();
	testcase->type = type;	
	testcase->sparam = sparam;
	testcase->lparam = lparam;
	testcase->monitor = monitor;
	testcase->usageBelow = usageBelow;
	
	listTestset.push_back(testcase);
}

void TestSet::AddAulLaunch(string appid){
	AddTestset(S_LAUNCH_AUL_LAUNCH, appid, 0, S_MONITOR_CPU_TOTAL, DEFAULT_USAGE_BELOW);
}

void TestSet::AddAulLaunch(string appid, const MonitorType monitor, const double usageBelow){
	AddTestset(S_LAUNCH_AUL_LAUNCH, appid, 0, monitor, usageBelow);
}

void TestSet::AddForkAndExec(string command) {
	AddTestset(S_LAUNCH_FORK_AND_EXEC, command, 0, DEFAULT_MONITOR_TYPE, DEFAULT_USAGE_BELOW);
}

void TestSet::AddForkAndExec(string command, const MonitorType monitor, const double usageBelow) {
	AddTestset(S_LAUNCH_FORK_AND_EXEC, command, 0, monitor, usageBelow);
}

void TestSet::AddQuickCommand(string command) {
	AddTestset(S_LAUNCH_QUICK_COMMAND, command, 0, DEFAULT_MONITOR_TYPE, DEFAULT_USAGE_BELOW);
}

void TestSet::AddSleep(long milliseconds) {
	AddTestset(S_LAUNCH_SLEEP, "", milliseconds, DEFAULT_MONITOR_TYPE, DEFAULT_USAGE_BELOW);
}

void TestSet::AddProcWrite(string path, string value){
	stringstream ss;
	
	ss << path << " " << value;
	AddTestset(S_LAUNCH_PROC_WRITE, ss.str(), 0, DEFAULT_MONITOR_TYPE, DEFAULT_USAGE_BELOW);
}

#if defined(TIZEN)
bool TestSet::AulLaunch(string appid) {
	bundle *b;
	
	b = bundle_create();
	//bundle_add(b,"type","SIM");
	
	if(aul_launch_app(appid, b) == AUL_R_OK)
		return true;
	else
		return false;
}
#else
bool TestSet::AulLaunch(string appid) {
	return true;
}
#endif

bool TestSet::StartTest() {
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
		string sparam = ((*it)->sparam);
		bool waitStabilized;
		bool waitChild;
		int childpid = -1;

		waitStabilized = (type == S_LAUNCH_FORK_AND_EXEC || type == S_LAUNCH_AUL_LAUNCH)? true: false;
		waitChild = (type == S_LAUNCH_QUICK_COMMAND)? true: false;

		if(type == S_LAUNCH_FORK_AND_EXEC || type == S_LAUNCH_QUICK_COMMAND) {
			//cout << "sparam: " << sparam << endl;
			childpid = launcher.forkAndExec(sparam, waitChild);
			if(childpid == -1) {
				error("Launch failed\n");
				return 0;
			}

			const double usageBelow = (*it)->usageBelow;
			const MonitorType monitor = (*it)->monitor;

			cout << "Fork Launching... " << sparam << " pid:" << childpid << endl;
			cout << "Wait until " << ((monitor == S_MONITOR_CPU_TOTAL)? "total": "proc") 
				 << " cpu usage below " << usageBelow << endl;
		}
		else if(type == S_LAUNCH_AUL_LAUNCH) {
			cout << "AUL Launching... " << sparam << endl;
			AulLaunch(sparam);
		}
		else if(type == S_LAUNCH_SLEEP) {			
			long timeToSleep = (*it)->lparam;

			cout << "Sleep for " << timeToSleep << "ms" << endl;
			ANNOTATE_CHANNEL_COLOR(50, ANNOTATE_LTGRAY, "Sleep");
			this_thread::sleep_for(chrono::milliseconds(timeToSleep));
			ANNOTATE_CHANNEL_END(50);
		}
		else if(type == S_LAUNCH_PROC_WRITE) {				
			string path, value;
			istringstream ss(sparam);
			
			ss >> path;
			ss >> value;

			Proc::WriteProc(path, value);
			cout << "written to " << path << " (" << value << ")" << endl;

			ANNOTATE_CHANNEL(50, sparam.c_str());
			ANNOTATE_CHANNEL_END(50);
		}

		if(waitStabilized) {
			const double usageBelow = (*it)->usageBelow;
			const MonitorType monitor = (*it)->monitor;
			
			userUsage.SetPID(childpid);
			userUsage.SetUsageBelow(usageBelow);
			userUsage.SetUsageMonitor(static_cast<CPUUsage::UsageMonitor>(monitor));

			kswapdUsage.SetPID(kswapdPid);

			ANNOTATE_CHANNEL_COLOR(50, ANNOTATE_BLUE, sparam.c_str());
			while(true) {
				userUsage.Tick();
				kswapdUsage.Tick();
				
				if(userUsage.IsMeasurable()) {
					memInfo.Read();
					PrintSystemUsage(userUsage, kswapdUsage, memInfo);

					if(userUsage.IsUsageStable()){
						cout << /*userUsage.GetProcName()*/ sparam << " is stablised at " << userUsage.StablisedAt() << "(s)" << endl << endl;
						break;
					}
				}

				this_thread::sleep_for(chrono::milliseconds(monitorPeriod));
			}
			ANNOTATE_CHANNEL_END(50);
		}
	}	
}
