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
		logger << MEMINFO_PATH << " is not present" << endl;
		return false;
	}

	while(getline(fileStat, line))  {
		string token;
		istringstream ss(line);
	
		ss >> token;

		unsigned int activefile, inactivefile;

		if(!token.compare(MEMINFO_TOTAL))
			ss >> total;
		else if(!token.compare(MEMINFO_FREE))
			ss >> free;
		else if(!token.compare(MEMINFO_AVAILABLE))
			ss >> available;
		else if(!token.compare(MEMINFO_SWAPTOTAL))
			ss >> swaptotal;
		else if(!token.compare(MEMINFO_SWAPFREE))
			ss >> swapfree;
		else if(!token.compare(MEMINFO_ACTIVEFILE))
			ss >> activefile;
		else if(!token.compare(MEMINFO_INACTIVEFILE))
			ss >> inactivefile;

		swapused = swaptotal - swapfree;
		pagecache = activefile + inactivefile;
	}

	return true;
}

