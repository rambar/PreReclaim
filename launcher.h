#include <string>

class Launcher {
public:	
	unsigned int forkAndExec(char* const, bool);

private:
	char** TokenizeCommand(char* const);
};
