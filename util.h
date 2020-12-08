#define MAX_BAR_LEN 256
#define MAX_FILES 10000
#define MAX_FILE_NAME 256
#define MAX_CLONES 256
#define CHECKSUMM_SIZE 256

#define DIG getDigits(MAX_FILES)

extern char *argv0;

static void verr(const char *fmt, va_list ap);
void warn(const char *fmt, ...);
void die(const char *fmt, ...);

int getCols(void);
int wipeEntry(int index);
void makeSHA256(const char *path, char *hash);
void progressBar(int cur, int total);
