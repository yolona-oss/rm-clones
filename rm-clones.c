#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "rm-clones.h"
#include "sha256/sha256.h"
#include "util.h"

struct fas {
	char file_name[MAX_FILE_NAME];
	char checksum[CHECKSUMM_SIZE];
};

struct faf {
	char file[MAX_FILE_NAME];
	char clone_file[MAX_CLONES][MAX_FILE_NAME];
};

static struct fas FilesAndChecks[MAX_FILES];
static struct faf FilesAndClones[MAX_FILES];

static void
usage()
{
	die("[-r|--recursive] [-q|--force] <DIR>...");
}

static int
preScan(const char *root, int recursive)
{
	char cur_file[PATH_MAX];
	char path[PATH_MAX];
	char rpath[PATH_MAX];
	int total_files;
	int tmp;

	struct stat fileStats;

	DIR *d;
	struct dirent *dir;

	if ((d = opendir(root)) == NULL) {
		warn("Cant open dir: %s", root);
		return -1;
	}

	if (d == NULL) {
		warn("Cant open directory: %s", root);
		return -1;
	}

	total_files = 0;
	while ((dir = readdir(d)) != NULL)
	{
		if (snprintf(cur_file, sizeof(cur_file),
				"%s", dir->d_name) == 0) {
			warn("Cant read file name");
			continue;
		}
		if ((snprintf(path, sizeof(char) * PATH_MAX,
				"%s/%s", root, cur_file)) < 1) {
			warn("Cant fill path");
			continue;
		}
		if (realpath(path, rpath) == NULL) {
			warn("realpath:");
			continue;
		}

		if (lstat(rpath, &fileStats) == -1) {
			warn("Cant get stats of file: %s", rpath);
			continue;
		}

		if (recursive == 1)
		{
			if (S_ISDIR(fileStats.st_mode) &&
					strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
			{
				snprintf(path, sizeof(path),
						"%s/%s", root, dir->d_name);
				if ((tmp = preScan(path, recursive)) > 0) {
					total_files += tmp;
				}
			} else if (dir->d_type != DT_DIR) {
				total_files++;
			}
		} else if (!S_ISDIR(fileStats.st_mode)) {
			total_files++;
		}
	}

	if (closedir(d) == -1) {
		warn("Cant close dir: %s", root);
		return -1;
	}

	return total_files;
}

static int
scan(const char *root, int processed, int recursive)
{
	int tmp;
	char cur_file[MAX_FILE_NAME];
	char path[PATH_MAX];
	char rpath[PATH_MAX];
	int iterations = 1;
	int total_files;
	char hash[65] = {0};

	struct stat fileStats;
	int fileStatus;

	DIR *d;
	struct dirent *dir;

	if ((d = opendir(root)) == NULL) {
		warn("Cant open dir: %s", root);
		return processed;
	}

	total_files = preScan(root, recursive);

	if (total_files == 0) {
		printf("Nothing to do in dir: %s\n", root);
		return processed;
	} else if (total_files == -1) {
		warn("An error resived while prescaning dir: %s", root);
		return processed;
	} else if (total_files > MAX_FILES) {
		total_files = MAX_FILES - 1;
		warn("Too many files in dir: %s", root);
		warn("Only %d will be processed", total_files);
	} else {
		printf("%d files found\n", total_files);
	}

	if ((d = opendir(root)) == NULL) {
		warn("Cant open dir: %s", root);
		return processed;
	}
	
	while ((dir = readdir(d)) != NULL && processed < MAX_FILES)
	{
		if (snprintf(cur_file, sizeof(cur_file),
				"%s", dir->d_name) == 0) {
			warn("Cant read file name");
			continue;
		}
		if ((snprintf(path, sizeof(char) * PATH_MAX,
				"%s/%s", root, cur_file)) < 1) {
			warn("Cant fill path");
			continue;
		}

		if (realpath(path, rpath) == NULL) {
			warn("realpath:");
			continue;
		}

		if (lstat(rpath, &fileStats) == -1) {
			warn("Cant get stats of file: %s", rpath);
			continue;
		}

		if (recursive == 1)
		{
			if (S_ISDIR(fileStats.st_mode) &&
					strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
			{
				processed = scan(path, processed, recursive);
			}
			else
			{
				if (lstat(path, &fileStats) == -1) {
					warn("Cant get stats of file: %s", path);
					continue;
				}

				makeSHA256(S_ISLNK(fileStats.st_mode) ? path : rpath, hash);
				if ((int)(hash) == HASH_DIR) {
					continue;
				}

				if (fillFileEntry(S_ISLNK(fileStats.st_mode) ? path : rpath, hash, processed) == -1) {
					warn("Cant fill entry №.%d", processed);
					continue;
				}

				processed++;
			}

			/* progressLine(root, processed, total_files); */
		}
		else
		{
			if (! S_ISDIR(fileStats.st_mode))
			{
				if (lstat(path, &fileStats) == -1) {
					warn("Cant get stats of file: %s", path);
					continue;
				}

				makeSHA256(S_ISLNK(fileStats.st_mode) ? path : rpath, hash);

				if (fillFileEntry(S_ISLNK(fileStats.st_mode) ? path : rpath, hash, processed) == -1) {
					warn("Cant fill entry №.%d", processed);
					continue;
				}

				processed++;

				progressLine(root, processed, total_files);
			}
		}
	}

	if (closedir(d) == -1) {
		warn("Cant close dir: %s", root);
		return processed;
	}

	return processed;
}

int
wipeEntry(int i)
{
	int ret = sprintf(FilesAndChecks[i].checksum,
			"%c", '\0');

	return ret;
}

static int
checkFileClones(int total, int force)
{
	printf("\nLooking for matches\n");

	int clones_count = 0;
	int cur, clone;
	for (cur = 0; cur < total; cur++)
	{
		for (clone = 0; clone < total; clone++)
		{
			if (clone != cur && strlen(FilesAndChecks[clone].checksum) != 0  &&
					strcmp(FilesAndChecks[cur].checksum,
						   FilesAndChecks[clone].checksum) == 0)
			{
				wipeEntry(clone);

				if (force == 1) {
					unlink(FilesAndChecks[clone].file_name);
				} else {
					printf("%s and %s\n", FilesAndChecks[cur].file_name,
							FilesAndChecks[clone].file_name);
				}

				clones_count++;
			}
		}
	}

	printf("Found %d clones\n", clones_count);

	return 0;
}

static int
fillFileEntry(const char *path, const char *hash, int i)
{
	if(snprintf(FilesAndChecks[i].file_name, sizeof(char) * MAX_FILE_NAME,
			"%s", path) == 0) {
		warn("Cant read file name");
		return -1;
	}
	
	if(snprintf(FilesAndChecks[i].checksum, sizeof(char) * CHECKSUMM_SIZE,
			"%s", hash) == 0) {
		warn("Cant get file hash");
		return -1;
	}

	return 0;
}

static void
createTableFile(int fd_clones_tab)
{
	;
}

int
main(int argc, char *argv[])
{
	int rflag = 0;
	int qflag = 0;

	argv0 = *argv;
	argv++;
	for (int i = 0; *argv && (*argv)[0] == '-' && (*argv)[1]; i++, argc--, argv++)
	{
		if ((*argv)[1] == '-') {
			*argv += 2;
		} else {
			(*argv)++;
		}

			if (strcmp(*argv, "r") == 0) {
				rflag = 1;
			} else if (strcmp(*argv, "recursive") == 0) {
				rflag = 1;
			} else if (strcmp(*argv, "q") == 0) {
				qflag = 1;
			} else if (strcmp(*argv, "force") == 0) {
				qflag = 1;
			} else if (strcmp(*argv, "h") == 0) {
				usage();
			} else if (strcmp(*argv, "help") == 0){
				usage();
			} else {
				usage();
			}
	}

	if (! *argv) {
		die("No directory path specified");
	}

	int processed;
	char root_dir[PATH_MAX];
	/* hendle all passed arguments */
	for (argc--; argc > 0; argv++, argc--)
	{
		sprintf(root_dir,
				"%s", *argv);
		if (root_dir[strlen(root_dir) - 1] == '/') {
			root_dir[strlen(root_dir) - 1] = '\0';
		}

		if ((processed = scan(root_dir, 0, rflag)) == -1) {
			warn("An error recived while scaning dir: %s", root_dir);
			/* skip */
			continue;
		}
		
		printf("processed: %d\n", processed);
		if (checkFileClones(processed, qflag) == -1) {
			warn("Some error while looking for file clones");
		}
	}
	
	return 0;
}
