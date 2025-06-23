#define _DEFAULT_SOURCE
#define FUSE_USE_VERSION 28

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char *fullpath(const char *path, const char *base);
void logger(const char *action, const char *path, const char *details);

static int xmp_getattr(const char *path, struct stat *stbuf);
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int xmp_open(const char *path, struct fuse_file_info *fi);
static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int xmp_truncate(const char *path, off_t size);
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int xmp_release(const char *path, struct fuse_file_info *fi);
static int xmp_unlink(const char *path);

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open = xmp_open,
    .read = xmp_read,
    .create = xmp_create,
    .write = xmp_write,
    .truncate = xmp_truncate,
    .release = xmp_release,
    .unlink = xmp_unlink,
};

#define SOURCE_DIR "source"

static char cwd[PATH_MAX];

int main(int argc, char *argv[]) {
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    char *fpath = fullpath("/", SOURCE_DIR);
    if (fpath == NULL) {
        perror("fullpath");
        return EXIT_FAILURE;
    }

    if (access(fpath, F_OK) == -1) {
        if (mkdir(fpath, 0755) == -1) {
            perror("mkdir");
            free(fpath);
            return EXIT_FAILURE;
        }
    }
    free(fpath);

    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}

char *fullpath(const char *path, const char *base) {
    char *full_path = malloc(strlen(cwd) + strlen(base) + strlen(path) + 3);
    if (full_path == NULL) return NULL;
    if (base == NULL || strlen(base) == 0) {
        sprintf(full_path, "%s/%s", cwd, path[0] == '/' ? path + 1 : path);
    } else {
        sprintf(full_path, "%s/%s/%s", cwd, base, path[0] == '/' ? path + 1 : path);
    }
    return full_path;
}

void logger(const char *action, const char *path, const char *details) {
    char *fpath = fullpath("history.log", "");
    FILE *fp = fopen(fpath, "a");
    if (fp == NULL) return;

    char time_str[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    if (details && strlen(details) > 0) {
        fprintf(fp, "[%s] %s: %s - %s\n", time_str, action, path, details);
    } else {
        fprintf(fp, "[%s] %s: %s\n", time_str, action, path);
    }

    fclose(fp);
}

static int xmp_getattr(const char *path, struct stat *stbuf) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;
    int res = lstat(fpath, stbuf);
    free(fpath);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    DIR *dp = opendir(fpath);
    if (dp == NULL) {
        free(fpath);
        return -errno;
    }

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        if (filler(buf, de->d_name, NULL, 0) != 0) {
            closedir(dp);
            free(fpath);
            return -ENOMEM;
        }
    }

    closedir(dp);
    free(fpath);

    logger("LIST", path, "mengakses directory");
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int fd = open(fpath, fi->flags);
    free(fpath);

    if (fd == -1) return -errno;
    fi->fh = fd;

    logger("OPEN", path, "membuka file");
    return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int fd = creat(fpath, mode);
    free(fpath);

    if (fd == -1) return -errno;
    fi->fh = fd;

    logger("CREATE", path, "membuat file baru");
    return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res = pwrite(fd, buf, size, offset);
    if (res == -1) return -errno;
    return res;
}

static int xmp_truncate(const char *path, off_t size) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int res = truncate(fpath, size);
    free(fpath);

    if (res == -1) return -errno;

    logger("EDIT", path, "mengubah file");

    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res = pread(fd, buf, size, offset);
    if (res == -1) return -errno;
    return res;
}

static int xmp_release(const char *path, struct fuse_file_info *fi) {
    int fd = fi->fh;
    if (fd > 0) close(fd);
    return 0;
}

static int xmp_unlink(const char *path) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int res = unlink(fpath);
    free(fpath);
    if (res == -1) return -errno;

    logger("DELETE", path, "menghapus file");
    return 0;
}