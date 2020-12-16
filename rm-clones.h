extern char *argv0;

static void usage();

static int fillFileEntry(const char *path, const char *hash, int i);

static int preScan(const char *path, int reqursive);
static int scan(const char *root, int processed, int reqursive);
static void createTableFile(int fd_clones_tab);
static int checkFileClones(int total, int force_flag);
