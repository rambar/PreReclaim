#include <iostream>
#include <thread>

#include "common.h"
#include "launcher.h"
#include "cpu_usage.h"
#include "proc.h"
#include "meminfo.h"

#include "testset.h"

#define KSWAPD_NAME 		"kswapd0"

using namespace std;

void TestSet::PrintSystemUsage(CPUUsage &userUsage, CPUUsage &kswapdUsage, const MemInfo &memInfo) {
	double cpuActive, cpuIdle;
	double userActive, kswapdActive;
	
	userUsage.GetCPUUsage(cpuActive, cpuIdle);
	userUsage.GetProcUsage(userActive);
	kswapdUsage.GetProcUsage(kswapdActive);

	cout << "frame:";
	cout.width(2);
	cout << userUsage.GetFrame() << "(";
	cout.precision(1);
	cout << userUsage.GetRunningTime() << ")";
	
	cout << " - cpu%:";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << cpuActive << "% /";
	
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << cpuIdle << "%";

	cout << " - pid(" << userUsage.GetPID() << "):";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << userActive << "%";

	cout << " - kswapd: ";
	cout << kswapdActive << "%";

	cout << " - avail Mem: ";
	cout << KBtoMB(memInfo.available) << "MB";
	
	cout << endl;
}

/*
void TestSet::SetMonitor(int target) {
	monitorTarget = target;
}*/

void TestSet::AddForkAndExec(const char* const path, const char* const option) {
	Testcase *testcase = new Testcase();
	testcase->type = S_LAUNCH_FORK_AND_EXEC;
	testcase->path = path;
	testcase->option = option;
	
	listTestset.push_back(testcase);
}

void TestSet::AddQuickCommand(const char* const path, const char* const option) {
	Testcase *testcase = new Testcase();
	testcase->type = S_LAUNCH_QUICK_COMMAND;
	testcase->path = path;
	testcase->option = option;
	
	listTestset.push_back(testcase);
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

		if(type == S_LAUNCH_FORK_AND_EXEC) {
			int childpid = launcher.forkAndExec(path, option, false);
			if(childpid == -1) {
				error("Launch failed\n");
				return 0;
			}

			userUsage.SetPID(childpid);
			kswapdUsage.SetPID(kswapdPid);

			while(true) {
				userUsage.Tick();
				kswapdUsage.Tick();
				
				if(userUsage.IsMeasurable()) {
					memInfo.Read();

					PrintSystemUsage(userUsage, kswapdUsage, memInfo);

					if(userUsage.IsUsageStable()){
						cout << "Stablised at " << userUsage.StablisedAt() << "(s)" << endl;
						break;
					}
				}

				this_thread::sleep_for(chrono::milliseconds(200));
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
