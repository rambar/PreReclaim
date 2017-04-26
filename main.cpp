#include <chrono>
#include <iostream>
#include <sstream> 
#include <string>

#include "common.h"
#include "testset.h"
#include "streamline_annotate.h"

//#define LAUNCH_FIREFOX 	"/usr/bin/firefox"
//#define LAUNCH_CHROME 	"/opt/google/chrome/chrome"
#define KILL_ALL		"/usr/bin/killall"
#define LAUNCH_GAME		"/usr/games/gnome-mahjongg"
#define LAUNCH_GIMP		"/usr/bin/gimp"

#define MALLOC_SIZE     4*1024*1024

#define PROC_EFM		"/proc/sys/vm/extra_free_kbytes"
#define SIZE_TO_RECLAIM "150000"
#define SIZE_ZERO		"0"

using namespace std;

ANNOTATE_DEFINE;

bool enablePreReclaim = false;

/*
void memalloc() {	
	char* buf;

	buf = (char*) malloc(MALLOC_SIZE);
	memset(buf, 0xff, MALLOC_SIZE);

	free(buf);
}*/

void AddPreJob(TestSet &testSet) {
	if(enablePreReclaim) {
		testSet.AddProcWrite(PROC_EFM, SIZE_TO_RECLAIM);
		testSet.AddSleep(3000);
		testSet.AddProcWrite(PROC_EFM, SIZE_ZERO);
	}
}

void AddPostJob(TestSet &testSet) {
	testSet.AddSleep(5000);
}

#ifdef TIZEN
void CreateTestJob(TestSet &testSet) {
	AddPreJob(testSet);
	testSet.AddForkAndExec ("/usr/bin/wascmd -r CZuqWL5gWu.pooq", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddAulLaunch ("org.tizen.browser", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddAulLaunch ("org.volt.mycontents", TestSet::S_MONITOR_CPU_TOTAL, 25.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddAulLaunch ("org.volt.search-all", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddAulLaunch ("org.volt.apps", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddAulLaunch ("evKhCgZelL.AmazonIgnitionLauncher2", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddForkAndExec ("/usr/bin/wascmd -r QizQxC7CUf.PlayMovies", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	AddPostJob(testSet);

	AddPreJob(testSet);
	testSet.AddForkAndExec ("/usr/bin/wascmd -r 9Ur5IzDKqV.TizenYouTube", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	AddPostJob(testSet);

	//testSet.AddForkAndExec ("/usr/bin/wascmd -r RN1MCdNq8t.Netflix", TestSet::S_MONITOR_CPU_TOTAL, 32.0);
	//testSet.AddForkAndExec ("/usr/bin/wascmd -r QizQxC7CUf.PlayMovies", TestSet::S_MONITOR_CPU_TOTAL, 30.0);
	//testSet.AddForkAndExec ("/usr/bin/wascmd -r YgmIZRmiap.MelOn", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	//testSet.AddForkAndExec ("/usr/bin/aul_test launch org.volt.apps", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
}
#else 
void CreateTestJob(TestSet &testSet) {

	testSet.SetMonitorPeriod(300);
	//testSet.AddForkAndExec (LAUNCH_GIMP);
	testSet.AddForkAndExec ("/opt/google/chrome/chrome google.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	testSet.AddForkAndExec ("/opt/google/chrome/chrome amazon.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	testSet.AddForkAndExec ("/opt/google/chrome/chrome ebay.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	testSet.AddForkAndExec ("/opt/google/chrome/chrome youtube.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	testSet.AddForkAndExec ("/opt/google/chrome/chrome naver.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
	testSet.AddForkAndExec ("/opt/google/chrome/chrome daum.net");
	testSet.AddForkAndExec ("/opt/google/chrome/chrome netflix.com");

	//testSet.AddForkAndExec (LAUNCH_GAME, NULL);
	//testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");
	//testSet.AddForkAndExec (LAUNCH_FIREFOX, "naver.com");
	//testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");	
}
#endif

int main(int argc, char *argv[]) {	
	ANNOTATE_SETUP;
	TestSet testSet;

	if(argc > 1 && !strcmp(argv[1], "-pr")) {
		enablePreReclaim = true;
	}

	CreateTestJob(testSet);
	testSet.StartTest();
	
	return 0;
}
