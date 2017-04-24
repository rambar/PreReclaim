#include <chrono>
#include <iostream>
#include <sstream> 
#include <string>

#include "common.h"
#include "testset.h"

#define LAUNCH_FIREFOX 	"/usr/bin/firefox"
#define LAUNCH_CHROME 	"/opt/google/chrome/chrome"
#define KILL_ALL		"/usr/bin/killall"
#define LAUNCH_GAME		"/usr/games/gnome-mahjongg"
#define LAUNCH_GIMP		"/usr/bin/gimp"

using namespace std;

int main(int argc, char *argv[]) {
	TestSet testSet;

	//testSet.AddForkAndExec (LAUNCH_GIMP, NULL);
	testSet.AddForkAndExec (LAUNCH_CHROME, "naver.com");
	testSet.AddForkAndExec (LAUNCH_CHROME, "youtube.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
//	testSet.AddForkAndExec (LAUNCH_CHROME, "google.com", TestSet::S_MONITOR_CPU_TOTAL, 50.0);
/*	testSet.AddForkAndExec (LAUNCH_CHROME, "amazon.com");
	testSet.AddForkAndExec (LAUNCH_CHROME, "ebay.com");
	testSet.AddForkAndExec (LAUNCH_CHROME, "daum.net");
	testSet.AddForkAndExec (LAUNCH_CHROME, "netflix.com");
*/
	/*
	testSet.AddForkAndExec (LAUNCH_GIMP, NULL);
	//testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");
	
	testSet.AddForkAndExec (LAUNCH_GAME, NULL);
	//testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");

	testSet.AddForkAndExec (LAUNCH_FIREFOX, "naver.com");
	//testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");
	*/
	
	testSet.StartTest();
	
	return 0;
}
