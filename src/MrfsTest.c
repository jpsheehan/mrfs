#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "Mrfs.h"

const char file54Text[] = "Hello world from file 54!\n";
const char file353Text[] = "Hello from 353!\n";
char * selectedText = NULL;

static int Mrfs_getattr(const char * path, struct stat * st)
{
	printf("getattr(\"%s\")\n", path);

	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time( NULL );
	st->st_mtime = time(NULL);

	if (strcmp(path, "/") == 0)
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	}
	else
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;

		if (strcmp(path, "/file54") == 0)
		{
			st->st_size = strlen(file54Text);
		}
		else if (strcmp(path, "/file353") == 0)
		{
			st->st_size = strlen(file353Text);
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

static int Mrfs_readdir(const char * path, void * buffer, fuse_fill_dir_t filler, off_t offset __attribute__((unused)), struct fuse_file_info * fi __attribute__((unused)))
{
	printf("readdir(\"%s\")\n", path);

	filler( buffer, ".", NULL, 0 );
	filler( buffer, "..", NULL, 0 );
	
	if (strcmp(path, "/") == 0 )
	{
		filler( buffer, "file54", NULL, 0 );
		filler( buffer, "file353", NULL, 0 );
	}
	
	return 0;
}

static int Mrfs_read(const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info *fi __attribute__((unused)))
{
	printf("read(\"%s\", size = %ld, offset = %lu)\n", path, size, offset);

	if (strcmp(path, "/file54") == 0)
	{
		selectedText = (char*)file54Text;
	}
	else if (strcmp(path, "/file353") == 0)
	{
		selectedText = (char*)file353Text;
	}
	else
	{
		return -1;
	}

	if ((uint64_t)offset > strlen(selectedText))
	{
		return -1;
	}
	
	if (offset + size > strlen(selectedText))
	{
		size = strlen(selectedText) - offset;
	}

	memcpy(buffer, selectedText + offset, size);
	
	return size;
}

static const struct fuse_operations mrfs_ops = {
	.getattr = Mrfs_getattr,
	.readdir = Mrfs_readdir,
	.read = Mrfs_read,
};

int main(int argc, char * argv[])
{
	return fuse_main(argc, argv, &mrfs_ops, NULL);
}
