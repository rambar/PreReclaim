#include <iostream>
#include <unistd.h>

#include "launcher.h"

using namespace std;

unsigned int Launcher::LaunchFireFox(){
	pid_t pid = fork();

	if (pid == -1) {
		// error, failed to fork()
	} 
	else if (pid > 0) {
		// parent
		return pid;
	}
	else {
		// we are the child
		execl(LAUNCHER_FIREFOX, NULL);
	}
}
