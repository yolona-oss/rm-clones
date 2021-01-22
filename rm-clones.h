extern char *argv0;

#define END_CLONES_LIST_SIGN "UNIF_FILE_NAME_ASDFASD"

static void usage();

static int fillFileEntry(const char *path, const char *hash, int i);
static int fillFileNClonesEntry(const char *file, const char *clone, int i, int j);

static int preScan(const char *path, int reqursive);
static int scan(const char *root, int processed, int reqursive);
static void createTableFile(int fd_clones_tab);
static int checkFileClones(int total, int force_flag);
static int wipeEntry(int i);
