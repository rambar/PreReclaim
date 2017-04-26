#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "common.h"
#include "launcher.h"

using namespace std;

char** Launcher::TokenizeCommand(string &command) {
	char** tokenArray;
	unsigned int i;
	
	istringstream ss(command);
	vector<string> tokenVector;
	string token;

	while (ss >> token)
		tokenVector.push_back(token);
	
	tokenArray = new char*[(tokenVector.size() + 1)];
	
	for(i = 0; i < tokenVector.size(); i++) {
		token = tokenVector.at(i);
		tokenArray[i] = new char[token.size() + 1];
		strcpy(tokenArray[i], token.c_str());
	}
	
	tokenArray[i] = 0;

	return tokenArray;
}

unsigned int Launcher::forkAndExec(string &command, bool waitchild) {
	if(command.length() == 0) {
		error("No command to fork and execute\n");
		return -1;
	}
	
	pid_t pid = fork();

	if (pid == -1) {
		// error, failed to fork()
		return -1;
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
		char** options = TokenizeCommand(command);

		/*
		int i=0;
		char* buf;
		while(true){
			buf = options[i++];
			if(buf == NULL) break;
			cout << i-1 << " : " << buf << endl << flush;
		}*/

		execv(options[0], options); //only return when error occurs.

		cout << "exec error: " << strerror(errno) << endl;
		delete [] options;
		delete options;
		
		exit(0);
	}
}
