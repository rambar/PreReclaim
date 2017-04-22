#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "launcher.h"

using namespace std;

unsigned int Launcher::forkAndExec(char* const path, char* const option, bool waitchild){
	//if(path.length() == 0) return -1;
	
	pid_t pid = fork();

	if (pid == -1) {
		// error, failed to fork()
	} 
	else if (pid > 0) {
		// parent
		if(waitchild) {
			int status;
			waitpid(pid, &status, 0);
		}
		return pid;
	}
	else {
		// we are the child
		char* const argv[3] = {path, option, NULL};
		execv(path, argv);
	}
}
