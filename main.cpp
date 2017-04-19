#include <chrono>
#include <iostream>
#include <thread>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include "common.h"
#include "proc.h"
#include "cpu_usage.h"
#include "meminfo.h"

using namespace std;

void PrintCPUUsage(const double cpu_active, const double cpu_idle, int pid, const double user_active, const double kswapd_active, MemInfo &memInfo)
{
	cout << "active: ";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(4);
	cout.precision(1);
	cout << cpu_active << "%";
	
	cout << " - idle: ";
	cout.setf(ios::fixed, ios::floatfield);
	cout.width(4);
	cout.precision(1);
	cout << cpu_idle << "%";

	cout << " - pid(" << pid << "): ";
	cout << user_active << "%";

	cout << " - kswapd: ";
	cout << kswapd_active << "%";

	cout << " - avail Mem: ";
	cout << KBtoMB(memInfo.available) << "MB";
	
	cout << endl;
}

int main(int argc, char *argv[]) {
	int userPid = -1;
	int kswapdPid;
	int first = true;

	const string KSWAPD_NAME("kswapd0");
	
	if(argc == 1) {
		cout << "usage: sys_stat [PID]" << endl;
		return 0;
	}

	userPid = stoi(argv[1]);

	kswapdPid = Proc::FindProcessName(KSWAPD_NAME);
	if(kswapdPid == -1) {
		error("kswapd is not enabled\n");
		return 0;
	}

	CPUUsage userUsage(userPid);
	CPUUsage kswapdUsage(kswapdPid);
	MemInfo memInfo;

	double cpuActive, cpuIdle;
	double userActive, kswapdActive;
	
	while(true) {
		if(first == true) {
			userUsage.Tick();
			kswapdUsage.Tick();
			
			first = false;
		}
		else {
			userUsage.Tick();
			kswapdUsage.Tick();

			userUsage.CountUsage();
			kswapdUsage.CountUsage();

			userUsage.GetCPUUsage(cpuActive, cpuIdle);
			userUsage.GetProcUsage(userActive);
			kswapdUsage.GetProcUsage(kswapdActive);

			memInfo.Read();

			PrintCPUUsage(cpuActive, cpuIdle, userPid, userActive, kswapdActive, memInfo);
		}

		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	
	return 0;
}
