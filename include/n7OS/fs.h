#ifndef __N7OS_FS_H__
#define __N7OS_FS_H__

#define FS_MAX_FILES 64
#define FS_MAX_PATH  64
#define FS_MAX_NAME  32
#define FS_MAX_DATA  4096

void init_fs(void);

int fs_sync(void);

int fs_resolve_path(const char *cwd, const char *path, char *out, int out_size);
int fs_complete(const char *cwd, const char *prefix, char *out, int out_size);
int fs_mkdir(const char *path);
int fs_is_dir(const char *path);

int fs_create(const char *path);
int fs_write(const char *path, const char *data);
int fs_append(const char *path, const char *data);
int fs_read(const char *path, char *buffer, int size);
int fs_delete(const char *path);
int fs_remove(const char *path);
int fs_copy(const char *src, const char *dst);
int fs_move(const char *src, const char *dst);
int fs_exists(const char *path);
int fs_size(const char *path);
int fs_ls(const char *path);
int fs_tree(const char *path);

#endif