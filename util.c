#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "util.h"
#include "sha256/sha256.h"

char *argv0;

static void
verr(const char *fmt, va_list ap)
{
	if (argv0 && strncmp(fmt, "usage", sizeof("usage") - 1)) {
		fprintf(stderr, "%s: ", argv0);
	}

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
}

void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
}

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);

	exit(1);
}

int
getCols(void)
{
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);

	return w.ws_col;
}

void
makeSHA256(const char *path, char *hash)
{
	FILE* file;
	static char buffer[1024];
	size_t size;
	struct sha256_buff buff;

	struct stat fileStats;

	if (stat(path, &fileStats) == -1) {
		warn("Cant get info about file: %s", path);
	}
	else
	{
		if (!S_ISDIR(fileStats.st_mode))
		{
			file = fopen(path, "rb");
			sha256_init(&buff);
			while (!feof(file)) {
				// Hash file by 1kb chunks, instead of loading into RAM at once
				size = fread(buffer, 1, 1024, file);
				sha256_update(&buff, buffer, size);
			}
			sha256_finalize(&buff);
			sha256_read_hex(&buff, hash);
		}
		else
		{
			sprintf(hash,
					"%c", '\0');
		}
	}
}

static int
getDigits(int long n)
{
	int d = 0;

	while ((n /= 10) != 0) {
		d++;
	} 

	return d;
}

void
progressBar(int cur, int total)
{
	int bp;
	char bar[MAX_BAR_LEN];
	int columns = getCols();
	int barLen = columns - 34 - DIG;
	if (barLen > MAX_BAR_LEN) {
		barLen = MAX_BAR_LEN;
	}
	int perc;

	perc = cur * 100 / total;

	// cant handle too wide screens
	for (bp = 0; bp < cur * barLen / total - 1; bp++)
		bar[bp] = '=';
	bar[bp] = '>';
	for (bp++; bp < barLen; bp++) {
		bar[bp] = '-';
	}
	bar[bp] = '\0';
	printf("\rScanning files. %*d remainig. [%s] %3d%%", DIG, total - cur, bar, perc);
	fflush(stdout);
}
