#define warn(fmt, args ...) printf("[WARN]" fmt, ##args);
#define info(fmt, args ...) printf("[INFO]" fmt, ##args);
#define error(fmt, args ...) printf("[ERROR]" fmt, ##args);

#define BtoMB(x)			((x)>>20)
#define KBtoMB(x)			((x)>>10)
