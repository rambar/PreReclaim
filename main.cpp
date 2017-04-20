#include <chrono>
#include <iostream>
#include <thread>
#include <sstream> 
#include <string>
#include <vector>

#include "launcher.h"
#include "common.h"
#include "proc.h"
#include "cpu_usage.h"
#include "meminfo.h"

using namespace std;

void PrintSystemUsage(CPUUsage &userUsage, CPUUsage &kswapdUsage, const MemInfo &memInfo) {
	double cpuActive, cpuIdle;
	double userActive, kswapdActive;
	
	userUsage.GetCPUUsage(cpuActive, cpuIdle);
	userUsage.GetProcUsage(userActive);
	kswapdUsage.GetProcUsage(kswapdActive);

	cout << "frame: ";
	cout.width(2);
	cout << userUsage.GetFrame() << "(";
	cout.precision(1);
	cout << userUsage.GetRunningTime() << ")";
	
	cout << " - active: ";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << cpuActive << "%";
	
	cout << " - idle: ";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(5);
	cout.precision(1);
	cout << cpuIdle << "%";

	cout << " - pid(" << userUsage.GetPID() << "): ";
	cout << userActive << "%";

	cout << " - kswapd: ";
	cout << kswapdActive << "%";

	cout << " - avail Mem: ";
	cout << KBtoMB(memInfo.available) << "MB";
	
	cout << endl;
}

int main(int argc, char *argv[]) {
	//int userPid = -1;
	int kswapdPid;
	int first = true;
	Launcher launcher;
	MemInfo memInfo;
	unsigned int childpid;
	const string KSWAPD_NAME("kswapd0");
	
	/*if(argc == 1) {
		cout << "usage: sys_stat [PID]" << endl;
		return 0;
	}*/

	//userPid = stoi(argv[1]);

	kswapdPid = Proc::FindProcessName(KSWAPD_NAME);
	if(kswapdPid == -1) {
		error("kswapd is not enabled\n");
		return 0;
	}

	childpid = launcher.LaunchFireFox();

	CPUUsage userUsage(childpid);
	CPUUsage kswapdUsage(kswapdPid);
	
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
	
	return 0;
}
