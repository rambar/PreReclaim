#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <dirent.h>

#include "common.h"
#include "proc.h"

using namespace std;

bool Proc::ReadProcessName(const int pid, string &name) {
	char path[PATH_MAX];
	string line;
	const string STR_NAME("Name:");

	snprintf(path, PATH_MAX, PROCSTATUS_PATH, pid);
	ifstream fileStat(path);

	if(!fileStat.good()){
		error("pid:%d is not present\n", pid);
		return false;
	}
	
	while(getline(fileStat, line))	{
		istringstream ss(line);
		string token;
	
		ss >> token;
		if(!token.compare(STR_NAME))	{
			ss >> name;
			return true;
		}
	}

	return false;
}

int Proc::FindProcessName(const string &find) {
	DIR *dir;
	struct dirent *ent;

	dir = opendir(PROC_PATH);
	while ((ent = readdir(dir)) != NULL) {
		string name;
		int pid;
		
		if(*ent->d_name < '0' || *ent->d_name > '9') continue;

		pid = stoi(ent->d_name);
		ReadProcessName(pid, name);

		if(!name.compare(find)) {
			(void)closedir(dir);
			return pid;
		}
	}

	(void)closedir(dir);
	return -1;
}

bool Proc::WriteProc(const string path, const string value){
	ofstream fileProc(path);

	if(!fileProc.is_open()){
		error("%s is not opened\n", path.c_str());
		return false;
	}

	fileProc << value;
	fileProc.close();
	
	return true;
}
