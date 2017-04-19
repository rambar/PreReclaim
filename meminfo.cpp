#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include "common.h"
#include "meminfo.h"

using namespace std;

bool MemInfo::Read() {
	ifstream fileStat(MEMINFO_PATH);
	string line;

	if(!fileStat.good()){
		error("%s is not present\n", MEMINFO_PATH);
		return false;
	}

	while(getline(fileStat, line))  {
		string token;
		istringstream ss(line);
	
		ss >> token;

		if(!token.compare(MEMINFO_TOTAL))
			ss >> total;
		else if(!token.compare(MEMINFO_FREE))
			ss >> free;
		else if(!token.compare(MEMINFO_AVAILABLE))
			ss >> available;
	}

	return true;
}

