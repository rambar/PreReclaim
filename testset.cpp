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

const string TestSet::Constants::EFM_PROC_PATH 		= "/proc/sys/vm/extra_free_kbytes";
const string TestSet::Constants::EFM_SIZE_TO_RECLAIM = "150000";
const string TestSet::Constants::EFM_SIZE_ZERO 		= "0";

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

void TestSet::AddAulLaunch(string appid, const MonitorType monitor, const double usageBelow){
	AddTestset(S_LAUNCH_AUL_LAUNCH, appid, 0, monitor, usageBelow);
}

void TestSet::AddForkAndExec(string command, const MonitorType monitor, const double usageBelow) {
	AddTestset(S_LAUNCH_FORK_AND_EXEC, command, 0, monitor, usageBelow);
}

void TestSet::AddQuickCommand(string command) {
	AddTestset(S_LAUNCH_QUICK_COMMAND, command, 0, S_MONITOR_UNDEFINED, 0);
}

void TestSet::AddSleep(long milliseconds) {
	AddTestset(S_LAUNCH_SLEEP, "", milliseconds, S_MONITOR_UNDEFINED, 0);
}

void TestSet::AddProcWrite(string path, string value){
	stringstream ss;
	
	ss << path << " " << value;
	AddTestset(S_LAUNCH_PROC_WRITE, ss.str(), 0, S_MONITOR_UNDEFINED, 0);
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
		cout << "Unable to parse:" << error->message << endl; 
		g_error_free (error);
		goto finish;
	}

	root = json_parser_get_root (parser);

	if (root == NULL || (JSON_NODE_TYPE (root) != JSON_NODE_ARRAY))
		goto finish;
	
	/*
	cout << "JSON_NODE_TYPE(root) :" << JSON_NODE_TYPE(root) << endl;
	cout << "JSON_NODE_ARRAY :" << JSON_NODE_ARRAY << endl;
	cout << "JSON_NODE_OBJECT :" << JSON_NODE_OBJECT << endl;
	*/
	
	array = json_node_get_array(root);
	arraySize = json_array_get_length (array);

	if(arraySize <= 0) 
		goto finish;
    
	for(guint i = 0; i < arraySize; i++) {
		string 				value;
		TestSet::LaunchType launchType;
		string 				command;
		TestSet::MonitorType monitor = TestSet::S_MONITOR_CPU_TOTAL;
		double 				usageBelow = 50.0f;
		
		JsonNode *arrayElem = json_array_get_element (array, i);
		JsonObject *object  = json_node_get_object (arrayElem);

		if(object == NULL) goto finish;

		value = json_object_get_string_member (object, Constants::JSON_LAUNCHTYPE.c_str());		

		if(!value.compare("ForkAndExec")) 
			launchType = S_LAUNCH_FORK_AND_EXEC;
		else if(!value.compare("QuickCommand"))
			launchType = S_LAUNCH_QUICK_COMMAND;
		else if(!value.compare("AulLaunch"))
			launchType = S_LAUNCH_AUL_LAUNCH;
		else if(!value.compare("Sleep"))
			launchType = S_LAUNCH_SLEEP;
		else if(!value.compare("ProcWrite"))
			launchType = S_LAUNCH_PROC_WRITE;

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

		if(launchType == S_LAUNCH_FORK_AND_EXEC){
			//pre-test
			if(PreReclaimEnabled()) {
				AddProcWrite(Constants::EFM_PROC_PATH, Constants::EFM_SIZE_TO_RECLAIM);
				AddSleep(3000);
				AddProcWrite(Constants::EFM_PROC_PATH, Constants::EFM_SIZE_ZERO);
			}

			//test
			AddForkAndExec(command, monitor, usageBelow); 

			//post-test
			AddSleep(5000);
		}
		else if(launchType == S_LAUNCH_AUL_LAUNCH){
			//pre-test
			if(PreReclaimEnabled()) {
				AddProcWrite(Constants::EFM_PROC_PATH, Constants::EFM_SIZE_TO_RECLAIM);
				AddSleep(3000);
				AddProcWrite(Constants::EFM_PROC_PATH, Constants::EFM_SIZE_ZERO);
			}

			//test
			AddAulLaunch(command, monitor, usageBelow); 

			//post-test
			AddSleep(5000);
		}
	}

	returnValue = true;

finish:
	g_object_unref (parser);

	return returnValue;
}

bool TestSet::StartTest() {
	// kswapd cpu usage measurement
	int kswapdPid = Proc::FindProcessName(Constants::KSWAPD_NAME);
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
		bool waitStabilized = false;
		bool waitChild;
		int childpid = -1;

		if(type == S_LAUNCH_FORK_AND_EXEC || type == S_LAUNCH_QUICK_COMMAND) {
			waitChild = (type == S_LAUNCH_QUICK_COMMAND)? true: false;
			
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

			waitStabilized = true;
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
