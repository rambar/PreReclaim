#include <iostream>
#include <sstream>
#include <time.h>

#include "common.h"
#include "testset.h"
#include "streamline_annotate.h"

//#define LAUNCH_FIREFOX 	"/usr/bin/firefox"
//#define LAUNCH_CHROME 	"/opt/google/chrome/chrome"
//#define KILL_ALL		"/usr/bin/killall"
//#define LAUNCH_GAME		"/usr/games/gnome-mahjongg"
//#define LAUNCH_GIMP		"/usr/bin/gimp"

using namespace std;

Logger logger;
ANNOTATE_DEFINE;

/*
#ifdef TIZEN
void CreateTestJob(TestSet &testSet) {

	FORK_EXEC ("/usr/bin/wascmd -r CZuqWL5gWu.pooq", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	AUL_LAUNCH ("org.tizen.browser", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AUL_LAUNCH ("org.volt.mycontents", TestSet::S_MONITOR_CPU_TOTAL, 25.0);
	AUL_LAUNCH ("org.volt.search-all", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AUL_LAUNCH ("org.volt.apps", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AUL_LAUNCH ("evKhCgZelL.AmazonIgnitionLauncher2", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	FORK_EXEC ("/usr/bin/wascmd -r QizQxC7CUf.PlayMovies", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	FORK_EXEC ("/usr/bin/wascmd -r 9Ur5IzDKqV.TizenYouTube", TestSet::S_MONITOR_CPU_TOTAL, 50.0);

	//FORK_EXEC ("/usr/bin/wascmd -r RN1MCdNq8t.Netflix", TestSet::S_MONITOR_CPU_TOTAL, 32.0);
	//FORK_EXEC ("/usr/bin/wascmd -r QizQxC7CUf.PlayMovies", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	//FORK_EXEC ("/usr/bin/wascmd -r YgmIZRmiap.MelOn", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	//FORK_EXEC ("/usr/bin/aul_test launch org.volt.apps", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
}
#endif
*/

void GetTimeString(string& str) {
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t rawtime = chrono::system_clock::to_time_t(now);
	struct tm * timeinfo = localtime(&rawtime);
	char buffer[80];

	strftime(buffer, 80, "%Y%m%d_%H%M%S", timeinfo);
	str = buffer;
}

int main(int argc, char *argv[]) {	
	ANNOTATE_SETUP;

	TestSet testset;
	string filename;

	if(argc == 1) {
		logger << "usage: ./run_test -file [filename] -pr" << endl;
		logger << "       -file : load testset from file" << endl;
		logger << "       -pr   : do pre-reclaim" << endl;	
		return 0;
	}
	
	for(int p = 1; p < argc; p++) {
		string arg = argv[p];
		
		if(!arg.compare("-pr")) {
			testset.SetPreReclaim(true);
		}
		else if(!arg.compare("-file")) {
			if(p + 1 >= argc) {
				//-file without filename
				logger << "filename is not specified" << endl;
				return 0;
			}
			filename = argv[++p];
		}
	}

	if(filename.length() == 0) {
		logger << "filename is not specified" << endl;
		return 0;
	}

	stringstream ss;
	if(testset.PreReclaimEnabled())
		ss << "pr_enabled_";
	else
		ss << "pr_disabled_";

	string currentTime;
	GetTimeString(currentTime);
	
	ss << currentTime << ".txt";
	logger.init(ss.str());

	if(testset.LoadFromFile(filename) == false){
		logger << "error loading json file (filename:" << filename.c_str() << ")" << endl;
		return 0;
	}

#ifndef TIZEN
	testset.SetMonitorPeriod(300);
#endif

	testset.StartTest();
	
	return 0;
}
