#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define MAX_FILES_ON_DIR 15
#define MAX_DIRS 15
#define MAX_NAME 100

typedef struct file {
	char name[MAX_NAME];
	char *content;
	int size;
	int mode;
	int uid;
	int gid;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
} file_t;

typedef struct dir {
	char name[MAX_NAME];
	file_t *files[MAX_FILES_ON_DIR];
	int file_amount;
	int mode;
	int uid;
	int gid;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
} dir_t;

typedef struct requestPath {
	char firstName[MAX_NAME];
	char secondName[MAX_NAME];
	bool isInsideDir;
} requestPath_t;

dir_t dirs_on_root[MAX_DIRS];
int dirs_on_root_size = 0;
dir_t root_dir;

char *fisopfs_filename;

bool using_default = true;
char *default_filename = "file.fisopfs";

void dumpToDisk();

char *
ft_strdup(const char *src)
{
	char *str;
	char *p;
	int len = 0;

	while (src[len])
		len++;
	str = malloc(len + 1);
	p = str;
	while (*src)
		*p++ = *src++;
	*p = '\0';
	return str;
}

requestPath_t
getRequestedPath(const char *path)
{
	requestPath_t requestedPath;
	requestedPath.isInsideDir = false;
	if (strcmp(path, "/") == 0) {
		strcpy(requestedPath.firstName, "/");
		return requestedPath;
	}
	char *pathCopy = ft_strdup(path);
	char *token = strtok(pathCopy, "/");
	if (token != NULL) {
		strcpy(requestedPath.firstName, token);
		token = strtok(NULL, "/");
		if (token != NULL) {
			strcpy(requestedPath.secondName, token);
			requestedPath.isInsideDir = true;
		}
	}
	free(pathCopy);
	return requestedPath;
}

bool
isInvalidPath(const char *path)
{
	int count = 0;
	for (int i = 0; i < strlen(path); i++) {
		if (path[i] == '/') {
			count++;
		}
	}

	return (count > 2);
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);


	if (strcmp(path, "/") == 0) {
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 1;
		return 0;
	}
	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			st->st_uid = root_dir.files[i]->uid;
			st->st_mode = root_dir.files[i]->mode;
			st->st_size = root_dir.files[i]->size;
			st->st_atime = root_dir.files[i]->atime.tv_sec;
			st->st_mtime = root_dir.files[i]->mtime.tv_sec;
			st->st_ctime = root_dir.files[i]->ctime.tv_sec;
			st->st_nlink = 1;
			return 0;
		}
		for (int i = 0; i < dirs_on_root_size; i++) {
			if (strcmp(dirs_on_root[i].name,
			           requestPath.firstName) != 0) {
				continue;
			}
			st->st_uid = dirs_on_root[i].uid;
			st->st_gid = dirs_on_root[i].gid;
			st->st_mode = dirs_on_root[i].mode;
			st->st_atime = dirs_on_root[i].atime.tv_sec;
			st->st_mtime = dirs_on_root[i].mtime.tv_sec;
			st->st_ctime = dirs_on_root[i].ctime.tv_sec;
			st->st_nlink = 1;
			return 0;
		}
		return -ENOENT;
	}

	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			st->st_uid = dirs_on_root[i].files[j]->uid;
			st->st_gid = dirs_on_root[i].files[j]->gid;
			st->st_mode = dirs_on_root[i].files[j]->mode;
			st->st_size = dirs_on_root[i].files[j]->size;
			st->st_atime = dirs_on_root[i].files[j]->atime.tv_sec;
			st->st_mtime = dirs_on_root[i].files[j]->mtime.tv_sec;
			st->st_ctime = dirs_on_root[i].files[j]->ctime.tv_sec;
			st->st_nlink = 1;
			return 0;
		}
		return -ENOENT;
	}
	return -ENOENT;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		for (int i = 0; i < dirs_on_root_size; i++) {
			filler(buffer, dirs_on_root[i].name, NULL, 0);
		}
		for (int i = 0; i < root_dir.file_amount; i++) {
			filler(buffer, root_dir.files[i]->name, NULL, 0);
		}
		return 0;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(path + 1, dirs_on_root[i].name) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			filler(buffer, dirs_on_root[i].files[j]->name, NULL, 0);
		}
		return 0;
	}

	return -ENOENT;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read(%s, %lu, %lu)\n", path, offset, size);

	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			if (offset + size > strlen(root_dir.files[i]->content))
				size = strlen(root_dir.files[i]->content) - offset;
			size = size > 0 ? size : 0;
			strncpy(buffer, root_dir.files[i]->content + offset, size);
			root_dir.files[i]->atime = ts;
			return size;
		}
		return -ENOENT;
	}

	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			if (offset + size >
			    strlen(dirs_on_root[i].files[j]->content))
				size = strlen(dirs_on_root[i].files[j]->content) -
				       offset;
			size = size > 0 ? size : 0;
			strncpy(buffer,
			        dirs_on_root[i].files[j]->content + offset,
			        size);
			dirs_on_root[i].files[j]->atime = ts;
			return size;
		}
	}
	return -ENOENT;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	if (!requestPath.isInsideDir) {
		if (root_dir.file_amount + 1 < MAX_FILES_ON_DIR) {
			int i = root_dir.file_amount;
			root_dir.files[i] = malloc(sizeof(file_t));
			strcpy(root_dir.files[i]->name, requestPath.firstName);
			root_dir.files[i]->content = malloc(sizeof(char) * 1);
			root_dir.files[i]->size = 1;
			root_dir.files[i]->content[0] = '\0';
			root_dir.files[i]->mode = mode;
			root_dir.files[i]->uid = getuid();
			root_dir.files[i]->gid = getgid();
			root_dir.files[i]->atime = ts;
			root_dir.files[i]->mtime = ts;
			root_dir.files[i]->ctime = ts;
			root_dir.file_amount += 1;
			return 0;
		}
		return -ENOMEM;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		if (dirs_on_root[i].file_amount + 1 < MAX_FILES_ON_DIR) {
			int j = dirs_on_root[i].file_amount;
			dirs_on_root[i].files[j] = malloc(sizeof(struct file));
			strcpy(dirs_on_root[i].files[j]->name,
			       requestPath.secondName);
			dirs_on_root[i].files[j]->content =
			        malloc(sizeof(char) * 1);
			dirs_on_root[i].files[j]->size = 1;
			dirs_on_root[i].files[j]->content[0] = '\0';
			dirs_on_root[i].files[j]->mode = mode;
			dirs_on_root[i].files[j]->uid = getuid();
			dirs_on_root[i].files[j]->gid = getgid();
			dirs_on_root[i].files[j]->atime = ts;
			dirs_on_root[i].files[j]->mtime = ts;
			dirs_on_root[i].files[j]->ctime = ts;
			dirs_on_root[i].file_amount += 1;
			return 0;
		}
		return -ENOMEM;
	}

	return -ENOMEM;
}

void
deleteIfExists(requestPath_t path)
{
	if (!path.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name, path.firstName) != 0) {
				continue;
			}
			free(root_dir.files[i]->content);
			free(root_dir.files[i]);
			for (int j = i; j < root_dir.file_amount - 1; j++) {
				root_dir.files[j] = root_dir.files[j + 1];
			}
			root_dir.file_amount -= 1;
			return;
		}
		return;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, path.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           path.secondName) != 0) {
				continue;
			}
			free(dirs_on_root[i].files[j]->content);
			free(dirs_on_root[i].files[j]);
			for (int k = j; k < dirs_on_root[i].file_amount - 1; k++) {
				dirs_on_root[i].files[k] =
				        dirs_on_root[i].files[k + 1];
			}
			dirs_on_root[i].file_amount -= 1;
			return;
		}
		return;
	}
}

static int
fisopfs_rename(const char *path, const char *newpath)
{
	if (isInvalidPath(path) || isInvalidPath(newpath)) {
		return -ENOENT;
	}
	if (strcmp(path, newpath) == 0) {
		return 0;
	}
	requestPath_t requestPath = getRequestedPath(path);
	requestPath_t newRequestPath = getRequestedPath(newpath);


	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			deleteIfExists(newRequestPath);
			strcpy(root_dir.files[i]->name, newRequestPath.firstName);
			return 0;
		}
		return -ENOENT;
	}
	int src_dir = -1, src_index = -1;
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		src_dir = i;
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			src_index = j;
		}
	}
	if (src_dir == -1 || src_index == -1) {
		return -ENOENT;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, newRequestPath.firstName) != 0) {
			continue;
		}
		if (dirs_on_root[i].file_amount == MAX_FILES_ON_DIR) {
			return -ENOSPC;
		}
		dirs_on_root[i].files[dirs_on_root[i].file_amount] =
		        dirs_on_root[src_dir].files[src_index];
		dirs_on_root[i].file_amount += 1;
		for (int j = src_index; j < dirs_on_root[src_dir].file_amount - 1;
		     j++) {
			dirs_on_root[src_dir].files[j] =
			        dirs_on_root[src_dir].files[j + 1];
		}
		dirs_on_root[src_dir].file_amount -= 1;
		strcpy(dirs_on_root[i].files[dirs_on_root[i].file_amount - 1]->name,
		       newRequestPath.secondName);
		return 0;
	}
	return -ENOENT;
}

static void
fisopfs_destroy(void *private_data)
{
	dumpToDisk();
	printf("[debug] Destroying filesystem in memory\n");
	for (int i = 0; i < root_dir.file_amount; i++) {
		free(root_dir.files[i]->content);
		free(root_dir.files[i]);
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			free(dirs_on_root[i].files[j]->content);
			free(dirs_on_root[i].files[j]);
		}
	}
	if (!using_default) {
		free(fisopfs_filename);
	}
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	if (isInvalidPath(path)) {
		return -ENOSPC;
	}
	requestPath_t requestPath = getRequestedPath(path);

	if (requestPath.isInsideDir || dirs_on_root_size >= MAX_DIRS) {
		return -ENOSPC;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) == 0) {
			return -EEXIST;
		}
	}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	int i = dirs_on_root_size;
	strcpy(dirs_on_root[i].name, path + 1);
	dirs_on_root[i].mode = mode | __S_IFDIR;
	dirs_on_root[i].uid = getuid();
	dirs_on_root[i].gid = getgid();
	dirs_on_root[i].file_amount = 0;
	dirs_on_root[i].atime = ts;
	dirs_on_root[i].mtime = ts;
	dirs_on_root[i].ctime = ts;
	dirs_on_root_size++;
	return 0;
}

static int
fisopfs_rmdir(const char *path)
{
	if (isInvalidPath(path)) {
		return -ENOENT;  // REVISAR
	}
	requestPath_t requestPath = getRequestedPath(path);

	if (requestPath.isInsideDir) {
		return -ENOENT;
	}
	int index = -1;
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, path + 1) == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return -ENOENT;
	}
	if (dirs_on_root[index].file_amount > 0) {
		return -ENOTEMPTY;
	}
	dirs_on_root[index] = dirs_on_root[dirs_on_root_size - 1];
	dirs_on_root_size--;
	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	if (requestPath.isInsideDir) {
		for (int i = 0; i < dirs_on_root_size; i++) {
			if (strcmp(dirs_on_root[i].name,
			           requestPath.firstName) != 0) {
				continue;
			}
			for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
				if (strcmp(dirs_on_root[i].files[j]->name,
				           requestPath.secondName) != 0) {
					continue;
				}
				free(dirs_on_root[i].files[j]->content);
				free(dirs_on_root[i].files[j]);
				dirs_on_root[i].files[j] =
				        dirs_on_root[i]
				                .files[dirs_on_root[i].file_amount - 1];
				dirs_on_root[i].file_amount--;
				return 0;
			}
			return -ENOENT;
		}
		return -ENOENT;
	}

	for (int i = 0; i < root_dir.file_amount; i++) {
		if (strcmp(root_dir.files[i]->name, requestPath.firstName) != 0) {
			continue;
		}
		free(root_dir.files[i]->content);
		free(root_dir.files[i]);
		root_dir.files[i] = root_dir.files[root_dir.file_amount - 1];
		root_dir.file_amount--;
		return 0;
	}

	return -ENOENT;
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write(path=\"%s\", buffer=\"%s\", size=%ld\n",
	       path,
	       buffer,
	       size);
	if (isInvalidPath(path)) {
		return -ENOSPC;
	}
	requestPath_t requestPath = getRequestedPath(path);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			if (offset > strlen(root_dir.files[i]->content)) {
				return -EFBIG;
			}
			if (offset + size > strlen(root_dir.files[i]->content)) {
				char *newContent =
				        realloc(root_dir.files[i]->content,
				                sizeof(char) * (offset + size));
				if (newContent == NULL) {
					return -ENOSPC;
				}
				root_dir.files[i]->content = newContent;
				root_dir.files[i]->size = offset + size;
			}
			memcpy(root_dir.files[i]->content + offset, buffer, size);
			root_dir.files[i]->mtime = ts;
			return size;
		}
		return -ENOENT;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			if (offset > strlen(dirs_on_root[i].files[j]->content)) {
				return -EFBIG;
			}
			if (offset + size >
			    strlen(dirs_on_root[i].files[j]->content)) {  // tal vez manejar si no hay error
				char *newContent =
				        realloc(dirs_on_root[i].files[j]->content,
				                sizeof(char) * (offset + size));
				if (newContent == NULL) {
					return -ENOSPC;
				}
				dirs_on_root[i].files[j]->content = newContent;
				dirs_on_root[i].files[j]->size = offset + size;
			}
			memcpy(dirs_on_root[i].files[j]->content + offset,
			       buffer,
			       size);
			dirs_on_root[i].files[j]->mtime = ts;
			dirs_on_root[i].mtime = ts;
			return size;
		}
		return -ENOENT;
	}
	return -ENOENT;
}

static int
fisopfs_utimens(const char *path, const struct timespec *tv)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			root_dir.files[i]->atime = tv[0];
			root_dir.files[i]->mtime = tv[1];
			return 0;
		}
		for (int i = 0; i < dirs_on_root_size; i++) {
			if (strcmp(dirs_on_root[i].name,
			           requestPath.firstName) != 0) {
				continue;
			}
			dirs_on_root[i].atime = tv[0];
			dirs_on_root[i].mtime = tv[1];
			return 0;
		}
		return -ENOENT;
	}

	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			dirs_on_root[i].files[j]->atime = tv[0];
			dirs_on_root[i].files[j]->mtime = tv[1];
			return 0;
		}
		return -ENOENT;
	}
	return -ENOENT;
}

static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			root_dir.files[i]->atime = ts;
			return 0;
		}
		return -ENOENT;
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			dirs_on_root[i].files[j]->atime = ts;
			return 0;
		}
		return -ENOENT;
	}
	return -ENOENT;
}

static int
fisopfs_truncate(const char *src, off_t offset)
{
	printf("[debug] truncate(%s, %ld)\n", src, offset);
	if (isInvalidPath(src)) {
		return -ENOENT;
	}

	requestPath_t requestPath = getRequestedPath(src);

	if (offset == 0) {
		offset = 1;
	}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			char *newContent = realloc(root_dir.files[i]->content,
			                           sizeof(char) * offset);
			if (newContent == NULL) {
				return -ENOMEM;
			}
			root_dir.files[i]->content = newContent;
			root_dir.files[i]->size = offset;
			root_dir.files[i]->mtime = ts;
			return 0;
		}
		return -ENOENT;
	}

	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			char *newContent =
			        realloc(dirs_on_root[i].files[j]->content,
			                sizeof(char) * offset);
			if (newContent == NULL) {
				return -ENOMEM;
			}
			dirs_on_root[i].files[j]->content = newContent;
			dirs_on_root[i].files[j]->size = offset;
			dirs_on_root[i].files[j]->mtime = ts;
			return 0;
		}
		return -ENOENT;
	}

	return -ENOENT;
}

static int
fisopfs_write_buf(const char *path,
                  struct fuse_bufvec *buf,
                  off_t offset,
                  struct fuse_file_info *fi)
{
	printf("[debug] Writing Buff %s, offset %li\n", path, offset);
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			size_t buffsize = buf->buf[0].size;
			char *newContent =
			        realloc(root_dir.files[i]->content,
			                sizeof(char) * (offset + buffsize));
			if (newContent == NULL) {
				return -ENOMEM;
			}
			root_dir.files[i]->content = newContent;
			memcpy(root_dir.files[i]->content + offset,
			       buf->buf[0].mem,
			       buf->buf[0].size);
			root_dir.files[i]->size = offset + buffsize;
			root_dir.files[i]->mtime = ts;
			return buffsize;
		}
		return -ENOENT;
	}

	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			size_t buffsize = buf->buf[0].size;
			char *newContent =
			        realloc(dirs_on_root[i].files[j]->content,
			                sizeof(char) * (offset + buffsize));
			if (newContent == NULL) {
				return -ENOMEM;
			}
			dirs_on_root[i].files[j]->content = newContent;
			memcpy(dirs_on_root[i].files[j]->content + offset,
			       buf->buf[0].mem,
			       buf->buf[0].size);
			dirs_on_root[i].files[j]->size = offset + buffsize;
			dirs_on_root[i].files[j]->mtime = ts;
			return buffsize;
		}
		return -ENOENT;
	}

	return -ENOENT;
}

static int
fisopfs_read_buf(const char *path,
                 struct fuse_bufvec **bufp,
                 size_t size,
                 off_t off,
                 struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read_buf(%s, %lu, %lu)\n", path, off, size);
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	if (!requestPath.isInsideDir) {
		for (int i = 0; i < root_dir.file_amount; i++) {
			if (strcmp(root_dir.files[i]->name,
			           requestPath.firstName) != 0) {
				continue;
			}
			if (off >= root_dir.files[i]->size) {
				return 0;
			}
			if (off + size > root_dir.files[i]->size) {
				size = root_dir.files[i]->size - off;
			}
			char *contentCopy = malloc(sizeof(char) * size);
			memcpy(contentCopy, root_dir.files[i]->content + off, size);
			*bufp = malloc(sizeof(struct fuse_bufvec));
			(*bufp)->buf[0].mem = contentCopy;
			(*bufp)->buf[0].size = size;
			(*bufp)->buf[0].pos = off;
			(*bufp)->count = 1;
			(*bufp)->idx = 0;
			root_dir.files[i]->atime = ts;
			return size;
		}
		return -ENOENT;
	}

	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) != 0) {
			continue;
		}
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			if (strcmp(dirs_on_root[i].files[j]->name,
			           requestPath.secondName) != 0) {
				continue;
			}
			if (off >= dirs_on_root[i].files[j]->size) {
				return 0;
			}
			if (off + size > dirs_on_root[i].files[j]->size) {
				size = dirs_on_root[i].files[j]->size - off;
			}
			char *contentCopy = malloc(sizeof(char) * size);
			memcpy(contentCopy,
			       dirs_on_root[i].files[j]->content + off,
			       size);
			*bufp = malloc(sizeof(struct fuse_bufvec));
			(*bufp)->buf[0].mem = contentCopy;
			(*bufp)->buf[0].size = size;
			(*bufp)->buf[0].pos = off;
			(*bufp)->count = 1;
			(*bufp)->idx = 0;
			dirs_on_root[i].files[j]->atime = ts;
			return size;
		}
		return -ENOENT;
	}

	return -ENOENT;
}


static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	dumpToDisk();
	return 0;
}

static int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	return 0;
}

static int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, "/") == 0) {
		return 0;
	}
	if (isInvalidPath(path)) {
		return -ENOENT;
	}
	requestPath_t requestPath = getRequestedPath(path);

	if (requestPath.isInsideDir) {
		return -ENOENT;
	}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	for (int i = 0; i < dirs_on_root_size; i++) {
		if (strcmp(dirs_on_root[i].name, requestPath.firstName) == 0) {
			dirs_on_root[i].atime = ts;
			return 0;
		}
	}

	return -ENOENT;
}

static int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	if (isInvalidPath(path)) {
		return -ENOENT;
	}

	requestPath_t requestPath = getRequestedPath(path);

	if (requestPath.isInsideDir) {
		return -ENOENT;
	}

	return 0;
}

void
loadFilesystem()
{
	FILE *file = fopen(fisopfs_filename, "rb");
	if (file == NULL) {
		printf("[debug] Error al abrir el archivo %s\n", fisopfs_filename);
		return;
	}
	printf("[debug] Cargando el sistema de archivos %s\n", fisopfs_filename);

	int totalRead = fread(&root_dir, sizeof(dir_t), 1, file);
	if (totalRead != 1) {
		printf("[debug] Error al leer el archivo %s\n", fisopfs_filename);
		return;
	}
	totalRead = fread(&dirs_on_root, sizeof(dir_t), MAX_DIRS, file);
	if (totalRead != MAX_DIRS) {
		printf("[debug] Error al leer el archivo %s\n", fisopfs_filename);
		return;
	}
	totalRead = fread(&dirs_on_root_size, sizeof(int), 1, file);
	if (totalRead != 1) {
		printf("[debug] Error al leer el archivo %s\n", fisopfs_filename);
		return;
	}

	for (int j = 0; j < root_dir.file_amount; j++) {
		root_dir.files[j] = malloc(sizeof(file_t));
		totalRead = fread(root_dir.files[j], sizeof(file_t), 1, file);
		if (totalRead != 1) {
			printf("[debug] Error al leer el archivo %s\n",
			       fisopfs_filename);
			return;
		}
		root_dir.files[j]->content =
		        malloc(sizeof(char) * root_dir.files[j]->size);
		totalRead = fread(root_dir.files[j]->content,
		                  sizeof(char),
		                  root_dir.files[j]->size,
		                  file);
		if (totalRead != root_dir.files[j]->size) {
			printf("[debug] Error al leer el archivo %s\n",
			       fisopfs_filename);
			return;
		}
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			dirs_on_root[i].files[j] = malloc(sizeof(file_t));
			totalRead = fread(dirs_on_root[i].files[j],
			                  sizeof(file_t),
			                  1,
			                  file);
			if (totalRead != 1) {
				printf("[debug] Error al leer el archivo %s\n",
				       fisopfs_filename);
				return;
			}
			dirs_on_root[i].files[j]->content = malloc(
			        sizeof(char) * dirs_on_root[i].files[j]->size);
			totalRead = fread(dirs_on_root[i].files[j]->content,
			                  sizeof(char),
			                  dirs_on_root[i].files[j]->size,
			                  file);
			if (totalRead != dirs_on_root[i].files[j]->size) {
				printf("[debug] Error al leer el archivo %s\n",
				       fisopfs_filename);
				return;
			}
		}
	}

	fclose(file);
}

void
dumpToDisk()
{
	FILE *file = fopen(fisopfs_filename, "wb");
	if (file == NULL) {
		printf("[debug] Error al abrir el archivo %s\n", fisopfs_filename);
		return;
	}
	printf("[debug] Guardando el sistema de archivos %s\n", fisopfs_filename);
	fwrite(&root_dir, sizeof(dir_t), 1, file);
	fwrite(dirs_on_root, sizeof(dir_t), MAX_DIRS, file);
	fwrite(&dirs_on_root_size, sizeof(int), 1, file);

	for (int j = 0; j < root_dir.file_amount; j++) {
		fwrite(root_dir.files[j], sizeof(file_t), 1, file);
		fwrite(root_dir.files[j]->content,
		       sizeof(char),
		       root_dir.files[j]->size,
		       file);
	}
	for (int i = 0; i < dirs_on_root_size; i++) {
		for (int j = 0; j < dirs_on_root[i].file_amount; j++) {
			fwrite(dirs_on_root[i].files[j], sizeof(file_t), 1, file);
			fwrite(dirs_on_root[i].files[j]->content,
			       sizeof(char),
			       dirs_on_root[i].files[j]->size,
			       file);
		}
	}
	fclose(file);
}

void
initFilesystem()
{
	root_dir.file_amount = 0;
}


static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	initFilesystem();
	loadFilesystem();
	return NULL;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.create = fisopfs_create,
	.rename = fisopfs_rename,
	.destroy = fisopfs_destroy,
	.write = fisopfs_write,
	.write_buf = fisopfs_write_buf,
	.read_buf = fisopfs_read_buf,
	.open = fisopfs_open,
	.opendir = fisopfs_opendir,
	.release = fisopfs_release,
	.releasedir = fisopfs_releasedir,
	.truncate = fisopfs_truncate,
	.utimens = fisopfs_utimens,
	.flush = fisopfs_flush,
	.init = fisopfs_init,
};

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Uso: %s <mountpoint> [filename] \n", argv[0]);
		return 1;
	}
	fisopfs_filename = default_filename;
	if (argc > 2 && argv[2][0] != '-') {
		using_default = false;
		fisopfs_filename = malloc(sizeof(char) * (strlen(argv[2]) + 1));
		strcpy(fisopfs_filename, argv[2]);
		strcat(fisopfs_filename, ".fisopfs");
		for (int i = 2; i < argc; i++) {
			argv[i] = argv[i + 1];
		}
		argc--;
	}
	return fuse_main(argc, argv, &operations, NULL);
}
