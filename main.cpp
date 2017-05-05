#include <iostream>

#include "common.h"
#include "testset.h"
#include "streamline_annotate.h"

//#define LAUNCH_FIREFOX 	"/usr/bin/firefox"
//#define LAUNCH_CHROME 	"/opt/google/chrome/chrome"
//#define KILL_ALL		"/usr/bin/killall"
//#define LAUNCH_GAME		"/usr/games/gnome-mahjongg"
//#define LAUNCH_GIMP		"/usr/bin/gimp"

using namespace std;

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

int main(int argc, char *argv[]) {	
	ANNOTATE_SETUP;

	TestSet testset;
	string filename;

	if(argc == 1) {
		cout << "usage: ./run_test -file [filename] -pr" << endl;
		cout << "       -file : load testset from file" << endl;
		cout << "       -pr   : do pre-reclaim" << endl;	
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
				cout << "filename is not specified" << endl;
				return 0;
			}
			filename = argv[++p];
		}
	}

	if(filename.length() == 0) {
		cout << "filename is not specified" << endl;
		return 0;
	}

	if(testset.LoadFromFile(filename) == false){
		error("error loading json file (filename:%s)\n", filename.c_str());
		return 0;
	}

#ifndef TIZEN
	testset.SetMonitorPeriod(300);
#endif

	testset.StartTest();
	
	return 0;
}
