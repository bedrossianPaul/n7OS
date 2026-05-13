#include <n7OS/fs.h>
#include <n7OS/ata.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define FS_MAGIC   0x3246534EUL
#define FS_VERSION 1
#define FS_LBA_START 1

typedef struct __attribute__((packed)) {
    int used;
    int is_dir;
    char path[FS_MAX_PATH];
    char parent[FS_MAX_PATH];
    char name[FS_MAX_NAME];
    uint32_t size;
    char data[FS_MAX_DATA];
} fs_entry_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    fs_entry_t entries[FS_MAX_FILES];
} fs_disk_t;

static fs_disk_t fs_disk;

static uint32_t fs_disk_sectors(void) {
    return (uint32_t)((sizeof(fs_disk) + 511) / 512);
}

static void fs_clear(void) {
    memset(&fs_disk, 0, sizeof(fs_disk));
    fs_disk.magic = FS_MAGIC;
    fs_disk.version = FS_VERSION;
}

static void trim_spaces(const char **path) {
    while (*path && (**path == ' ' || **path == '\t')) {
        (*path)++;
    }
}

static int split_components(const char *path, char parts[][FS_MAX_NAME], int max_parts) {
    int count = 0;
    const char *p = path;
    while (*p) {
        while (*p == '/') p++;
        if (*p == '\0') break;

        char part[FS_MAX_NAME];
        int i = 0;
        while (*p != '\0' && *p != '/' && i < FS_MAX_NAME - 1) {
            part[i++] = *p++;
        }
        part[i] = '\0';

        if (strcmp(part, "") == 0 || strcmp(part, ".") == 0) {
            continue;
        }
        if (strcmp(part, "..") == 0) {
            if (count > 0) count--;
            continue;
        }

        if (count >= max_parts) {
            return -1;
        }
        strcpy(parts[count], part);
        count++;
    }
    return count;
}

int fs_resolve_path(const char *cwd, const char *path, char *out, int out_size) {
    if (!out || out_size <= 0) {
        return -1;
    }
    out[0] = '\0';
    if (!path) {
        return -1;
    }

    char components[16][FS_MAX_NAME];
    int count = 0;

    trim_spaces(&path);
    if (path[0] == '\0') {
        return -1;
    }

    if (path[0] != '/' && cwd && cwd[0] != '\0') {
        const char *c = cwd;
        if (c[0] == '/') {
            c++;
        }
        count = split_components(c, components, 16);
        if (count < 0) {
            return -1;
        }
    }

    if (path[0] == '/') {
        count = 0;
        path++;
    }

    while (*path) {
        while (*path == '/') path++;
        if (*path == '\0') break;

        char part[FS_MAX_NAME];
        int i = 0;
        while (*path != '\0' && *path != '/' && i < FS_MAX_NAME - 1) {
            part[i++] = *path++;
        }
        part[i] = '\0';

        if (strcmp(part, "") == 0 || strcmp(part, ".") == 0) {
            continue;
        }
        if (strcmp(part, "..") == 0) {
            if (count > 0) count--;
            continue;
        }

        if (count >= 16) {
            return -1;
        }
        strcpy(components[count], part);
        count++;
    }

    if (count == 0) {
        strncpy(out, "/", out_size - 1);
        out[out_size - 1] = '\0';
        return 0;
    }

    int pos = 0;
    out[pos++] = '/';
    for (int i = 0; i < count; i++) {
        int len = strlen(components[i]);
        if (pos + len + 1 >= out_size) {
            return -1;
        }
        memcpy(out + pos, components[i], len);
        pos += len;
        if (i + 1 < count) {
            out[pos++] = '/';
        }
    }
    out[pos] = '\0';
    return 0;
}

int fs_complete(const char *cwd, const char *prefix, char *out, int out_size) {
    if (!out || out_size <= 0 || !prefix) {
        return -1;
    }
    out[0] = '\0';

    int is_absolute = (prefix[0] == '/');

    char parent_prefix[FS_MAX_PATH];
    char partial[FS_MAX_NAME];
    parent_prefix[0] = '\0';
    partial[0] = '\0';

    const char *slash = strrchr(prefix, '/');
    if (slash) {
        int parent_len = (int)(slash - prefix);
        if (parent_len >= FS_MAX_PATH) parent_len = FS_MAX_PATH - 1;
        if (parent_len == 0 && is_absolute) {
            strcpy(parent_prefix, "/");
        } else {
            memcpy(parent_prefix, prefix, parent_len);
            parent_prefix[parent_len] = '\0';
        }
        strncpy(partial, slash + 1, sizeof(partial) - 1);
        partial[sizeof(partial) - 1] = '\0';
    } else {
        strncpy(partial, prefix, sizeof(partial) - 1);
        partial[sizeof(partial) - 1] = '\0';
    }

    char base_dir[FS_MAX_PATH];
    if (slash) {
        if (fs_resolve_path(cwd, parent_prefix[0] ? parent_prefix : "/", base_dir, sizeof(base_dir)) < 0) {
            return -1;
        }
    } else {
        if (cwd && cwd[0] != '\0') {
            strncpy(base_dir, cwd, sizeof(base_dir) - 1);
            base_dir[sizeof(base_dir) - 1] = '\0';
        } else {
            strncpy(base_dir, "/", sizeof(base_dir) - 1);
            base_dir[sizeof(base_dir) - 1] = '\0';
        }
    }

    int matches = 0;
    char chosen[FS_MAX_PATH];
    chosen[0] = '\0';

    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_disk.entries[i].used || strcmp(fs_disk.entries[i].parent, base_dir) != 0) {
            continue;
        }
        if (strncmp(fs_disk.entries[i].name, partial, strlen(partial)) != 0) {
            continue;
        }
        matches++;
        if (matches == 1) {
            if (is_absolute) {
                if (strcmp(parent_prefix, "/") == 0 || parent_prefix[0] == '\0') {
                    snprintf(chosen, sizeof(chosen), "/%s", fs_disk.entries[i].name);
                } else {
                    snprintf(chosen, sizeof(chosen), "%s/%s", parent_prefix, fs_disk.entries[i].name);
                }
            } else if (parent_prefix[0] != '\0') {
                snprintf(chosen, sizeof(chosen), "%s/%s", parent_prefix, fs_disk.entries[i].name);
            } else {
                strncpy(chosen, fs_disk.entries[i].name, sizeof(chosen) - 1);
                chosen[sizeof(chosen) - 1] = '\0';
            }
            if (fs_disk.entries[i].is_dir && strlen(chosen) + 1 < sizeof(chosen)) {
                strcat(chosen, "/");
            }
        }
    }

    if (matches == 1) {
        strncpy(out, chosen, out_size - 1);
        out[out_size - 1] = '\0';
    }
    return matches;
}

static void fs_path_name(const char *path, char *name) {
    int len = strlen(path);
    int i = len - 1;
    while (i >= 0 && path[i] == '/') i--;
    while (i >= 0 && path[i] != '/') i--;
    i++;
    int j = 0;
    while (path[i] != '\0' && j < FS_MAX_NAME - 1) {
        name[j++] = path[i++];
    }
    name[j] = '\0';
}

static void fs_parent_path(const char *path, char *parent, int parent_size) {
    int len = strlen(path);
    while (len > 1 && path[len - 1] == '/') len--;
    int i = len - 1;
    while (i > 0 && path[i] != '/') i--;
    if (i <= 0) {
        strncpy(parent, "/", parent_size - 1);
        parent[parent_size - 1] = '\0';
        return;
    }
    if (i >= parent_size) {
        parent[0] = '\0';
        return;
    }
    memcpy(parent, path, i);
    parent[i] = '\0';
}

static int find_file(const char *path) {
    if (!path || path[0] == '\0') {
        return -1;
    }
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_disk.entries[i].used && strcmp(fs_disk.entries[i].path, path) == 0) {
            return i;
        }
    }
    return -1;
}

static int alloc_file(const char *path) {
    if (!path || path[0] == '\0' || strcmp(path, "/") == 0) {
        return -1;
    }
    if (find_file(path) >= 0) {
        return -1;
    }
    char parent[FS_MAX_PATH];
    fs_parent_path(path, parent, sizeof(parent));
    if (strcmp(parent, "/") != 0 && find_file(parent) < 0) {
        return -1;
    }

    char name[FS_MAX_NAME];
    fs_path_name(path, name);
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_disk.entries[i].used) {
            fs_disk.entries[i].used = 1;
            fs_disk.entries[i].is_dir = 0;
            strcpy(fs_disk.entries[i].path, path);
            strcpy(fs_disk.entries[i].parent, parent);
            strcpy(fs_disk.entries[i].name, name);
            fs_disk.entries[i].size = 0;
            fs_disk.entries[i].data[0] = '\0';
            return i;
        }
    }
    return -1;
}

static int fs_load_from_disk(void) {
    uint8_t *raw = (uint8_t *)&fs_disk;
    uint32_t sectors = fs_disk_sectors();

    for (uint32_t i = 0; i < sectors; i++) {
        if (ata_read_sector(FS_LBA_START + i, raw + (i * 512)) < 0) {
            return -1;
        }
    }

    if (fs_disk.magic != FS_MAGIC || fs_disk.version != FS_VERSION) {
        return -1;
    }

    return 0;
}

int fs_sync(void) {
    uint8_t *raw = (uint8_t *)&fs_disk;
    uint32_t sectors = fs_disk_sectors();

    fs_disk.magic = FS_MAGIC;
    fs_disk.version = FS_VERSION;

    for (uint32_t i = 0; i < sectors; i++) {
        if (ata_write_sector(FS_LBA_START + i, raw + (i * 512)) < 0) {
            return -1;
        }
    }

    return 0;
}

static int alloc_dir(const char *path) {
    if (!path || path[0] == '\0' || strcmp(path, "/") == 0) {
        return -1;
    }
    if (find_file(path) >= 0) {
        return -1;
    }

    char parent[FS_MAX_PATH];
    fs_parent_path(path, parent, sizeof(parent));
    if (strcmp(parent, "/") != 0 && find_file(parent) < 0) {
        return -1;
    }

    char name[FS_MAX_NAME];
    fs_path_name(path, name);
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_disk.entries[i].used) {
            fs_disk.entries[i].used = 1;
            fs_disk.entries[i].is_dir = 1;
            strcpy(fs_disk.entries[i].path, path);
            strcpy(fs_disk.entries[i].parent, parent);
            strcpy(fs_disk.entries[i].name, name);
            fs_disk.entries[i].size = 0;
            fs_disk.entries[i].data[0] = '\0';
            return i;
        }
    }
    return -1;
}

static void build_child_path(const char *parent, const char *name, char *out, int out_size) {
    if (strcmp(parent, "/") == 0) {
        strncpy(out, "/", out_size - 1);
        out[out_size - 1] = '\0';
        strncat(out, name, out_size - strlen(out) - 1);
        return;
    }
    strncpy(out, parent, out_size - 1);
    out[out_size - 1] = '\0';
    strncat(out, "/", out_size - strlen(out) - 1);
    strncat(out, name, out_size - strlen(out) - 1);
}

static void clear_entry(int idx) {
    fs_disk.entries[idx].used = 0;
    fs_disk.entries[idx].is_dir = 0;
    fs_disk.entries[idx].name[0] = '\0';
    fs_disk.entries[idx].path[0] = '\0';
    fs_disk.entries[idx].parent[0] = '\0';
    fs_disk.entries[idx].data[0] = '\0';
    fs_disk.entries[idx].size = 0;
}

static int remove_entry_recursive_idx(int idx) {
    if (idx < 0 || idx >= FS_MAX_FILES || !fs_disk.entries[idx].used) {
        return -1;
    }

    char path[FS_MAX_PATH];
    strncpy(path, fs_disk.entries[idx].path, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_disk.entries[i].used && strcmp(fs_disk.entries[i].parent, path) == 0) {
            if (remove_entry_recursive_idx(i) < 0) {
                return -1;
            }
        }
    }

    clear_entry(idx);
    return 0;
}

static int copy_entry_recursive_idx(int src_idx, const char *dst_path) {
    if (src_idx < 0 || src_idx >= FS_MAX_FILES || !fs_disk.entries[src_idx].used) {
        return -1;
    }

    if (fs_disk.entries[src_idx].is_dir) {
        if (alloc_dir(dst_path) < 0) {
            return -1;
        }

        for (int i = 0; i < FS_MAX_FILES; i++) {
            if (fs_disk.entries[i].used && strcmp(fs_disk.entries[i].parent, fs_disk.entries[src_idx].path) == 0) {
                char child_dst[FS_MAX_PATH];
                build_child_path(dst_path, fs_disk.entries[i].name, child_dst, sizeof(child_dst));
                if (copy_entry_recursive_idx(i, child_dst) < 0) {
                    return -1;
                }
            }
        }
        return 0;
    }

    if (alloc_file(dst_path) < 0) {
        return -1;
    }
    int dst_idx = find_file(dst_path);
    if (dst_idx < 0) {
        return -1;
    }
    int len = (int)fs_disk.entries[src_idx].size;
    if (len >= FS_MAX_DATA) {
        len = FS_MAX_DATA - 1;
    }
    memcpy(fs_disk.entries[dst_idx].data, fs_disk.entries[src_idx].data, len);
    fs_disk.entries[dst_idx].data[len] = '\0';
    fs_disk.entries[dst_idx].size = fs_disk.entries[src_idx].size;
    return 0;
}

int fs_is_dir(const char *path) {
    if (strcmp(path, "/") == 0) {
        return 1;
    }
    int idx = find_file(path);
    if (idx < 0) {
        return 0;
    }
    return fs_disk.entries[idx].is_dir != 0;
}

void init_fs(void) {
    ata_init();

    if (fs_load_from_disk() < 0) {
        fs_clear();

        fs_mkdir("home");
        fs_mkdir("home/user");
        fs_create("readme.txt");
        fs_write("readme.txt", "Bienvenue dans le FS persistant de n7OS.\n");
        fs_create("hello.txt");
        fs_write("hello.txt", "Hello from n7OS filesystem!\n");
        (void)fs_sync();
    }
}

int fs_mkdir(const char *path) {
    if (alloc_dir(path) < 0) {
        return -1;
    }
    return fs_sync() == 0 ? 0 : -1;
}

int fs_exists(const char *path) {
    return find_file(path) >= 0;
}

int fs_create(const char *path) {
    return (alloc_file(path) >= 0 && fs_sync() == 0) ? 0 : -1;
}

int fs_write(const char *path, const char *data) {
    int idx = find_file(path);
    if (idx < 0 || fs_disk.entries[idx].is_dir) {
        idx = alloc_file(path);
        if (idx < 0) {
            return -1;
        }
    }

    int len = strlen(data);
    if (len >= FS_MAX_DATA) {
        len = FS_MAX_DATA - 1;
    }
    memcpy(fs_disk.entries[idx].data, data, len);
    fs_disk.entries[idx].data[len] = '\0';
    fs_disk.entries[idx].size = (uint32_t)len;
    return fs_sync() == 0 ? len : -1;
}

int fs_append(const char *path, const char *data) {
    int idx = find_file(path);
    if (idx < 0 || fs_disk.entries[idx].is_dir) {
        idx = alloc_file(path);
        if (idx < 0) {
            return -1;
        }
    }

    int len = strlen(data);
    int room = FS_MAX_DATA - 1 - (int)fs_disk.entries[idx].size;
    if (room <= 0) {
        return (int)fs_disk.entries[idx].size;
    }
    if (len > room) {
        len = room;
    }
    memcpy(fs_disk.entries[idx].data + fs_disk.entries[idx].size, data, len);
    fs_disk.entries[idx].size += (uint32_t)len;
    fs_disk.entries[idx].data[fs_disk.entries[idx].size] = '\0';
    return fs_sync() == 0 ? (int)fs_disk.entries[idx].size : -1;
}

int fs_read(const char *path, char *buffer, int size) {
    int idx = find_file(path);
    if (idx < 0 || size <= 0 || fs_disk.entries[idx].is_dir) {
        return -1;
    }

    int len = (int)fs_disk.entries[idx].size;
    if (len >= size) {
        len = size - 1;
    }
    memcpy(buffer, fs_disk.entries[idx].data, len);
    buffer[len] = '\0';
    return len;
}

int fs_delete(const char *path) {
    return fs_remove(path);
}

int fs_remove(const char *path) {
    int idx = find_file(path);
    if (idx < 0) {
        return -1;
    }
    if (remove_entry_recursive_idx(idx) < 0) {
        return -1;
    }
    return fs_sync();
}

int fs_copy(const char *src, const char *dst) {
    int src_idx = find_file(src);
    if (src_idx < 0 || dst == 0 || dst[0] == '\0') {
        return -1;
    }
    if (find_file(dst) >= 0) {
        return -1;
    }
    if (copy_entry_recursive_idx(src_idx, dst) < 0) {
        return -1;
    }
    return fs_sync();
}

int fs_move(const char *src, const char *dst) {
    if (fs_copy(src, dst) < 0) {
        return -1;
    }
    if (fs_remove(src) < 0) {
        return -1;
    }
    return fs_sync();
}

int fs_size(const char *path) {
    int idx = find_file(path);
    if (idx < 0) {
        return -1;
    }
    return (int)fs_disk.entries[idx].size;
}

static void list_dir(const char *dir) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_disk.entries[i].used && strcmp(fs_disk.entries[i].parent, dir) == 0) {
            if (fs_disk.entries[i].is_dir) {
                printf("[%s]/\n", fs_disk.entries[i].name);
            } else {
                printf("%s\t(%d bytes)\n", fs_disk.entries[i].name, (int)fs_disk.entries[i].size);
            }
        }
    }
}

static void tree_dir(const char *dir, int depth) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_disk.entries[i].used && strcmp(fs_disk.entries[i].parent, dir) == 0) {
            for (int d = 0; d < depth; d++) {
                printf("  ");
            }
            if (fs_disk.entries[i].is_dir) {
                printf("[%s]/\n", fs_disk.entries[i].name);
                tree_dir(fs_disk.entries[i].path, depth + 1);
            } else {
                printf("%s\n", fs_disk.entries[i].name);
            }
        }
    }
}

int fs_ls(const char *path) {
    char dir[FS_MAX_PATH];
    if (path == 0 || path[0] == '\0') {
        strncpy(dir, "/", sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
    } else {
        strncpy(dir, path, sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
    }

    if (strcmp(dir, "/") != 0 && !fs_is_dir(dir)) {
        printf("Not a directory\n");
        return -1;
    }

    printf("-- %s --\n", dir);
    for (int i = 0; i < FS_MAX_FILES; i++) {
        (void)i;
    }
    list_dir(dir);
    return 0;
}

int fs_tree(const char *path) {
    char dir[FS_MAX_PATH];
    if (path == 0 || path[0] == '\0') {
        strncpy(dir, "/", sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
    } else {
        strncpy(dir, path, sizeof(dir) - 1);
        dir[sizeof(dir) - 1] = '\0';
    }

    if (strcmp(dir, "/") != 0 && !fs_is_dir(dir)) {
        printf("Not a directory\n");
        return -1;
    }

    printf("%s\n", dir);
    tree_dir(dir, 1);
    return 0;
}