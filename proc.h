#define PROC_PATH			"/proc"
#define PROCSTATUS_PATH		"/proc/%d/status"
#define PROC_EFM_PATH		"/proc/sys/vm/extra_free_kbytes"

class Proc {
public:
	static bool ReadProcessName(int pid, std::string &name);
	static int FindProcessName(const std::string &find);
	static bool WriteProc(std::string path, std::string value);
};
