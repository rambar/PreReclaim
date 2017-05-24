#include <iostream>
#include <thread>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

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

const string TestSet::Constants::KSWAPD_NAME 		= "kswapd0";

const string TestSet::Constants::JSON_LAUNCHTYPE 	= "launchType";
const string TestSet::Constants::JSON_COMMAND 		= "command";
const string TestSet::Constants::JSON_MONITOR 		= "monitor";
const string TestSet::Constants::JSON_USAGEBELOW 	= "usageBelow";
const string TestSet::Constants::JSON_WAITSTABLIZED	= "waitStablized";
const string TestSet::Constants::JSON_PROCNAME		= "procname";

const string TestSet::Constants::EFM_PROC_PATH 		= "/proc/sys/vm/extra_free_kbytes";
const string TestSet::Constants::ZERO 				= "0";

using namespace std;

void TestSet::PrintSystemUsageHeader() {
	logger << "time\t";
	logger << "tot.cpu\t";
	logger << "usr.cpu\t";
	logger << "kswapd\t";
	logger << "mem.av\t";
	logger << "mem.fr\t";
	logger << "mem.sw\n";
}

void TestSet::PrintSystemUsage(CPUUsage &userUsage, CPUUsage &kswapdUsage, const MemInfo &memInfo) {
	double cpuActive, cpuIdle;
	double userActive, kswapdActive;
	
	userUsage.GetCPUUsage(cpuActive, cpuIdle);
	userUsage.GetProcUsage(userActive);
	kswapdUsage.GetProcUsage(kswapdActive);

	logger.precision(1);
	logger << left << userUsage.GetRunningTime();
	logger << "\t";
	
	logger.setf(ios::fixed, ios::floatfield);
	logger.precision(0);
	logger << cpuActive;
	logger << "\t";

	if(userUsage.GetUsageMonitor() == CPUUsage::S_USAGE_USER_PROC) {
		if(userUsage.GetPID() == -1) {
			logger << " \"" << userUsage.GetProcName() << "\" not found";
		}
		else {
			logger.setf(ios::fixed, ios::floatfield);
			logger.precision(0);
			logger << userActive;
			logger << "\t";
		}
	}
	else{
		logger << 0;
		logger << "\t";
	}
	
	logger.precision(0);
	logger << kswapdActive;
	logger << "\t";

	logger << KBtoMB(memInfo.available);
	logger << "\t";
	
	logger << KBtoMB(memInfo.free);
	logger << "\t";
	
	logger << KBtoMB(memInfo.swapused);
	logger << "\t";
	
	logger << endl;
}

void TestSet::AddTestset(const LaunchType type, string sparam, long lparam, const MonitorType monitor, const double usageBelow, string procname, bool waitStablized) {
	Testcase *testcase = new Testcase();
	testcase->type = type;
	testcase->sparam = sparam;
	testcase->lparam = lparam;
	testcase->monitor = monitor;
	testcase->usageBelow = usageBelow;
	testcase->procname = procname;
	testcase->waitStablized = waitStablized;
	
	listTestset.push_back(testcase);
}

void TestSet::AddAulLaunch(string appid, const MonitorType monitor, const double usageBelow, string procname, bool waitStablized){
	AddTestset(S_LAUNCH_AUL_LAUNCH, appid, 0, monitor, usageBelow, procname, waitStablized);
}

void TestSet::AddForkAndExec(string command, const MonitorType monitor, const double usageBelow, string procname, bool waitStablized) {
	AddTestset(S_LAUNCH_FORK_AND_EXEC, command, 0, monitor, usageBelow, procname, waitStablized);
}

void TestSet::AddSleep(long milliseconds) {
	AddTestset(S_LAUNCH_SLEEP, "", milliseconds, S_MONITOR_UNDEFINED, 0, "", false);
}

void TestSet::AddPreReclaim(string value){
	AddTestset(S_LAUNCH_PRE_RECLAIM, value, 0, S_MONITOR_UNDEFINED, 0, "", false);
}

void TestSet::AddWaitStablised(const double usageBelow) {
	AddTestset(S_LAUNCH_WAIT_STABLISED, "", 0, S_MONITOR_CPU_TOTAL, usageBelow, "", true);
}

#if defined(TIZEN)
bool TestSet::AulLaunch(string appid) {
	bundle *b;
	
	b = bundle_create();
	//bundle_add(b,"type","SIM");
	
	if(aul_launch_app(appid.c_str(), b) == AUL_R_OK)
		return true;
	else
		return false;
}
#else
bool TestSet::AulLaunch(string appid) {
	return true;
}
#endif

bool TestSet::LoadFromFile(string &filename) {
	JsonParser *parser;
	JsonNode *root;
	GError *error = NULL;
	JsonArray* array;
	guint arraySize;
	bool returnValue = false;

	if(filename.length() == 0) return false;
	
	parser = json_parser_new ();
	json_parser_load_from_file (parser, filename.c_str(), &error);
	if (error) {
		logger << "Unable to parse:" << error->message << endl; 
		g_error_free (error);
		goto finish;
	}

	root = json_parser_get_root (parser);

	if (root == NULL || (JSON_NODE_TYPE (root) != JSON_NODE_ARRAY))
		goto finish;
	
	array = json_node_get_array(root);
	arraySize = json_array_get_length (array);

	if(arraySize <= 0) 
		goto finish;
    
	for(guint i = 0; i < arraySize; i++) {
		string 		value;
		LaunchType 	launchType;
		string 		command;
		MonitorType monitor = S_MONITOR_USER_PROC;
		double 		usageBelow = 50.0f;
		string 		procname;
		bool		waitStablized = true;
		
		JsonNode *arrayElem = json_array_get_element (array, i);
		JsonObject *object  = json_node_get_object (arrayElem);

		if(object == NULL) goto finish;

		value = json_object_get_string_member (object, Constants::JSON_LAUNCHTYPE.c_str());		

		if(!value.compare("ForkAndExec")) 
			launchType = S_LAUNCH_FORK_AND_EXEC;
		else if(!value.compare("AulLaunch"))
			launchType = S_LAUNCH_AUL_LAUNCH;
		else if(!value.compare("Sleep"))
			launchType = S_LAUNCH_SLEEP;
		else if(!value.compare("PreReclaim"))
			launchType = S_LAUNCH_PRE_RECLAIM;

		if(json_object_has_member (object, Constants::JSON_COMMAND.c_str())) {
			command = json_object_get_string_member (object, Constants::JSON_COMMAND.c_str());
		}

		if(json_object_has_member (object, Constants::JSON_MONITOR.c_str())) {
			value = json_object_get_string_member (object, Constants::JSON_MONITOR.c_str());

			if(!value.compare("CPU_TOTAL"))
				monitor = S_MONITOR_CPU_TOTAL;
			else if(!value.compare("USER_PROC"))
				monitor = S_MONITOR_USER_PROC;
		}

		if(json_object_has_member (object, Constants::JSON_USAGEBELOW.c_str())) {
			usageBelow = json_object_get_double_member (object, Constants::JSON_USAGEBELOW.c_str());
		}

		if(json_object_has_member (object, Constants::JSON_WAITSTABLIZED.c_str())) {
			waitStablized = json_object_get_boolean_member (object, Constants::JSON_WAITSTABLIZED.c_str());
		}

		if(json_object_has_member (object, Constants::JSON_PROCNAME.c_str())) {
			procname = json_object_get_string_member (object, Constants::JSON_PROCNAME.c_str());
		}

		if(launchType == S_LAUNCH_FORK_AND_EXEC) {
			//pre-test
			AddWaitStablised(50);
			
			//test
			AddForkAndExec(command, monitor, usageBelow, procname, waitStablized); 

			//post-test
			AddSleep(3000);
		}
		else if(launchType == S_LAUNCH_AUL_LAUNCH) {
			//pre-test
			AddWaitStablised(50);
			
			//test
			AddAulLaunch(command, monitor, usageBelow, procname, waitStablized); 

			//post-test
			AddSleep(3000);
		}
		else if(launchType == S_LAUNCH_PRE_RECLAIM) {
			if(PreReclaimEnabled()) {
				AddPreReclaim(command);
				AddSleep(5000);
				AddPreReclaim(Constants::ZERO);
				AddSleep(1000);
			}
		}
	}

	returnValue = true;

finish:
	g_object_unref (parser);

	return returnValue;
}

void TestSet::FillProcname(string &procname, string &sparam){
	if(procname.length() == 0) {
		istringstream ss(sparam);
		string token;

		ss >> token; //split " ";
		ss.str(token);
		
		while(getline(ss, token, '/')); //split "/";
		
		procname = token.substr(0, 15);
	}
}

bool TestSet::StartTest() {
	// kswapd cpu usage measurement
	int kswapdPid = Proc::FindProcessName(Constants::KSWAPD_NAME);
	if(kswapdPid == -1) {
		logger << "kswapd is not enabled" << endl;
		return 0;
	}

	for (vector<Testcase *>::iterator it = listTestset.begin() ; it != listTestset.end(); ++it) {
		Launcher launcher;
		CPUUsage userUsage;
		CPUUsage kswapdUsage;
		MemInfo memInfo;

		//get parameters from testcase data struct
		LaunchType type = (*it)->type;
		string sparam = (*it)->sparam;
		bool waitStablized = (*it)->waitStablized;
		const double usageBelow = (*it)->usageBelow;
		const MonitorType monitor = (*it)->monitor;
		string procname = (*it)->procname;

		bool waitChild = false;
		int childpid = -1;

		//set kswapd pid
		kswapdUsage.SetPID(kswapdPid);

		if(type == S_LAUNCH_FORK_AND_EXEC) {
			//tick first-time before launching application
			userUsage.Tick();
			kswapdUsage.Tick();
			ANNOTATE_CHANNEL_COLOR(50, ANNOTATE_BLACK, "fork_exec");
			
			childpid = launcher.forkAndExec(sparam, waitChild);
			if(childpid == -1) {
				logger << "Launch failed" << endl;
				return 0;
			}

			ANNOTATE_CHANNEL_END(50);

			logger << endl << "Fork Launching... \"" << sparam << "\" pid:" << childpid << endl;
			logger << "Wait until " << ((monitor == S_MONITOR_CPU_TOTAL)? "tot.cpu": "usr.cpu") 
				 << " CPU usage < " << usageBelow << endl;
		}
		else if(type == S_LAUNCH_AUL_LAUNCH) {
			//tick first-time before launching application
			userUsage.Tick();
			kswapdUsage.Tick();

			ANNOTATE_CHANNEL_COLOR(50, ANNOTATE_BLACK, "aul_launch");
			
			AulLaunch(sparam);

			ANNOTATE_CHANNEL_END(50);

			logger << endl << "AUL Launching... \"" << sparam << "\"" << endl;
			logger << "Wait until " << ((monitor == S_MONITOR_CPU_TOTAL)? "tot.cpu": "usr.cpu") 
				 << " CPU usage < " << usageBelow << endl;
		}
		else if(type == S_LAUNCH_SLEEP) {
			long timeToSleep = (*it)->lparam;

			logger << endl << "Sleep for " << timeToSleep << "ms" << endl;
			ANNOTATE_CHANNEL_COLOR(50, ANNOTATE_LTGRAY, "Sleep");

			kswapdUsage.SetPID(kswapdPid);
			userUsage.SetUsageMonitor(CPUUsage::S_USAGE_CPU_TOTAL);

			PrintSystemUsageHeader();
			while(true) {
				this_thread::sleep_for(chrono::milliseconds(monitorPeriod));
				if(userUsage.GetRunningTime() > (timeToSleep / 1000)) break;
				
				userUsage.Tick();
				kswapdUsage.Tick();

				if(userUsage.IsMeasurable()) {
					memInfo.Read();	
					PrintSystemUsage(userUsage, kswapdUsage, memInfo);
				}
			}
			
			ANNOTATE_CHANNEL_END(50);
		}
		else if(type == S_LAUNCH_PRE_RECLAIM) {			
			logger << endl << "Pre-Reclaim: " << sparam << endl;
			Proc::WriteProc(Constants::EFM_PROC_PATH, sparam);				

			ANNOTATE_CHANNEL(50, sparam.c_str());
			ANNOTATE_CHANNEL_END(50);
		}
		else if(type == S_LAUNCH_WAIT_STABLISED) {
			logger << endl << "Wait stablised tot.cpu < " << usageBelow << endl;
		}

		if(waitStablized) {
			FillProcname(procname, sparam);
			
			//userUsage.SetPID(childpid);
			userUsage.SetProcName(procname);
			userUsage.SetUsageBelow(usageBelow);
			userUsage.SetUsageMonitor(static_cast<CPUUsage::UsageMonitor>(monitor));

			ANNOTATE_CHANNEL_COLOR(50, ANNOTATE_BLUE, sparam.c_str());
			PrintSystemUsageHeader();
			
			while(true) {
				this_thread::sleep_for(chrono::milliseconds(monitorPeriod));
				
				userUsage.Tick();
				kswapdUsage.Tick();
				
				if(userUsage.IsMeasurable()) {
					memInfo.Read();
					PrintSystemUsage(userUsage, kswapdUsage, memInfo);

					if(userUsage.IsUsageStable()) {
						logger.precision(1);
						logger << /*userUsage.GetProcName()*/ sparam << " is stablised at " << userUsage.StablisedAt() << "(s)" << endl << endl;
						break;
					}
				}
			}
			ANNOTATE_CHANNEL_END(50);
		}
	}	
}
