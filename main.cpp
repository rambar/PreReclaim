#include <chrono>
#include <iostream>
#include <sstream> 
#include <string>

#include "common.h"
#include "testset.h"

#define LAUNCH_FIREFOX 	"/usr/bin/firefox"
#define KILL_ALL		"/usr/bin/killall"
#define LAUNCH_GAME		"/usr/games/gnome-mahjongg"

using namespace std;

int main(int argc, char *argv[]) {
	TestSet testSet;

	testSet.AddForkAndExec (LAUNCH_GAME, NULL);
	testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");
	
	testSet.AddForkAndExec (LAUNCH_GAME, NULL);
	testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");

	testSet.AddForkAndExec (LAUNCH_GAME, NULL);
	testSet.AddQuickCommand(KILL_ALL, "gnome-mahjongg");
	
	testSet.StartTest();
	
	return 0;
}
