#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>

#include <sys/types.h>
#include <dirent.h>

#include "common.h"
#include "cpu_usage.h"
#include "proc.h"

using namespace std;

CPUUsage::CPUUsage(const int pid) {
	this->pid = pid;
}

void CPUUsage::SetPID(const unsigned int pid) { 
	this->pid = pid; 
}

void CPUUsage::SetProcName(string &name) { 
	this->pname = name;
}

/*
void CPUUsage::GetProcName(string &name) {
	if(pid == -1) {
		name = "PID_UNDEFINED";
		return;
	}
	
	Proc::ReadProcessName(pid, name); 
}
*/

void CPUUsage::Tick() {
	if(pid == -1 && usageMonitor == S_USAGE_USER_PROC) {
		pid = Proc::FindProcessName(pname);
	}
	
	if(firstTick) {
		ReadCPUStats();
		ReadProcStats();
		tickStart = chrono::system_clock::now();

		firstTick = false;
	}
	else {
		//save it to prev
		copy(curCpuTime, curCpuTime + NUM_CPU_TIME_STATES, prevCpuTime);
		copy(curProcTimes, curProcTimes + NUM_PROCESS_TIME_STATES, prevProcTimes);
		
		ReadCPUStats();
		ReadProcStats();
		CountUsage();

		tickLast = chrono::system_clock::now();

		double usageToCheck = (usageMonitor == S_USAGE_CPU_TOTAL)? cpuActive: userActive;
		CheckUsageStable(usageToCheck);

		measurable = true;
		frame++;
	}
}

size_t CPUUsage::GetIdleTime(size_t *cpu_times) {
    return  cpu_times[S_IDLE] +
            cpu_times[S_IOWAIT];
}

size_t CPUUsage::GetActiveTime(size_t *cpu_times) {
    return  cpu_times[S_USER] +
			cpu_times[S_NICE] +
			cpu_times[S_SYSTEM] +
			cpu_times[S_IRQ] +
			cpu_times[S_SOFTIRQ] +
			cpu_times[S_STEAL] +
			cpu_times[S_GUEST] +
			cpu_times[S_GUEST_NICE];
}

bool CPUUsage::ReadCPUStats() {
	string line;
	const string STR_CPU("cpu");

	ifstream fileStat(PROCSTAT_PATH);

	if(!fileStat.good()){
		logger << PROCSTAT_PATH << " is not present" << endl;
		return false;
	}
	
	while(getline(fileStat, line))  {
		string token;
		istringstream ss(line);
	
		ss >> token;

		if(!token.compare(STR_CPU)) {
			for(int i = 0; i < NUM_CPU_TIME_STATES; ++i)
				ss >> curCpuTime[i];
		}
	}

	fileStat.close();

	return true;
}	

bool CPUUsage::ReadProcStats() {
	char path[PATH_MAX];
	string line;
	unsigned long utime, ktime;
	
	if(pid == -1) {
		curProcTimes[S_UTIME] = -1;
		curProcTimes[S_KTIME] = -1;
		return false;
	}

	snprintf(path, PATH_MAX, PROCPID_PATH, pid);
	ifstream fileStat(path);

	if(!fileStat.good()){
		logger << "pid:" << pid << " is not present" << endl;
		return false;
	}

	while(getline(fileStat, line)) {
		string temp;
		istringstream ss(line);

		for(int i=1; i<=13; i++)
			ss >> temp;
		
		ss >> utime >> ktime;
	}

	curProcTimes[S_UTIME] = utime;
	curProcTimes[S_KTIME] = ktime;

	fileStat.close();

	return true;
}

void CPUUsage::CountUsage() {
	activeTime	= static_cast<float>(GetActiveTime(curCpuTime) - GetActiveTime(prevCpuTime));
	idleTime	= static_cast<float>(GetIdleTime(curCpuTime) - GetIdleTime(prevCpuTime));
	totalTime	= activeTime + idleTime;

	cpuActive = (100.f * activeTime / totalTime);
	cpuIdle = (100.f * idleTime / totalTime);
	
	double user_time = curProcTimes[S_UTIME] - prevProcTimes[S_UTIME];
	double sys_time = curProcTimes[S_KTIME] - prevProcTimes[S_KTIME];

	//user proc pid is yet to be set
	if(prevProcTimes[S_UTIME] == -1)
		userActive = 0.0f;
	else
		userActive = (100.f * (user_time + sys_time) / totalTime);
}

void CPUUsage::GetCPUUsage(double &cpuActive, double &cpuIdle) {
	cpuActive = this->cpuActive;
	cpuIdle = this->cpuIdle;
}

void CPUUsage::GetProcUsage(double &userActive) {
	userActive = this->userActive;
}

double CPUUsage::TimeBetween(std::chrono::system_clock::time_point a, std::chrono::system_clock::time_point b) {
	return ((std::chrono::duration<double>)(b - a)).count();
}

bool CPUUsage::IsUsageStable() { 
	if(usageMonitor == S_USAGE_CPU_TOTAL)
		return isUsageStable == S_USAGE_STABLIZED;
	else if(usageMonitor == S_USAGE_USER_PROC)
		return isUsageStable == S_USAGE_STABLIZED && pid != -1;
}

void CPUUsage::CheckUsageStable(const double usage) {	
	if(isUsageStable == S_USAGE_ABOVE && usage < usageBelow) {
		//first time usage gets below
		isUsageStable = S_USAGE_BELOW;
		tickUsageBelow = tickLast;
	}
	else if(isUsageStable == S_USAGE_BELOW) {
		if(usage >= usageBelow) {
			isUsageStable = S_USAGE_ABOVE;
		}
		else{
			if(TimeBetween(tickUsageBelow, tickLast) >= secToStay){
				isUsageStable = S_USAGE_STABLIZED;
			}
		}
	}
}

