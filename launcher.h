#include <string>

class Launcher {
public:	
	unsigned int forkAndExec(char* const path, char* const option, bool waitchild);
};
