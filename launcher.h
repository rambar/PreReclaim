#include <string>

class Launcher {
public:	
	unsigned int forkAndExec(std::string &, bool);

private:
	char** TokenizeCommand(std::string &);
};
