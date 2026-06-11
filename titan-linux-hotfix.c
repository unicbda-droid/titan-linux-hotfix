#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH 65536
#define MAX_FILE_SIZE 10485760

static int replace_in_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0 || size > MAX_FILE_SIZE) {
        fclose(f);
        fprintf(stderr, "Error: file %s too large (%ld)\n", path, size);
        return -1;
    }
    rewind(f);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "Error: out of memory\n");
        return -1;
    }
    size_t read_size = fread(buf, 1, size, f);
    buf[read_size] = '\0';
    fclose(f);

    char *match = buf;
    int replaced = 0;
    while ((match = strstr(match, "-nopie")) != NULL) {
        memcpy(match, "-no-pie", 7);
        match += 7;
        replaced = 1;
    }

    if (!replaced) {
        free(buf);
        return 0;
    }

    f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Error: cannot write %s: %s\n", path, strerror(errno));
        free(buf);
        return -1;
    }
    fwrite(buf, 1, read_size, f);
    fclose(f);
    free(buf);
    return 1;
}

static int remove_pch(const char *dir)
{
    DIR *d = opendir(dir);
    if (!d) return 0;

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(d)) != NULL) {
        const char *ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".pch") == 0 || strcmp(ext, ".gch") == 0)) {
            char path[MAX_PATH];
            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
            if (remove(path) == 0) {
                printf("  Removed stale PCH: %s\n", path);
                count++;
            }
        }
    }
    closedir(d);
    return count;
}

static int process_nbproject(const char *nbproject, int *total)
{
    const char *files[] = {
        "Makefile-Debug.mk",
        "Makefile-Release.mk",
        "configurations.xml",
        NULL
    };

    printf("\n  nbproject: %s\n", nbproject);
    for (int i = 0; files[i]; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", nbproject, files[i]);
        printf("    %s: ", files[i]);
        int result = replace_in_file(path);
        if (result > 0) {
            printf("fixed\n");
            (*total)++;
        } else if (result == 0) {
            printf("ok\n");
        } else {
            printf("skipped\n");
        }
    }

    int pch = remove_pch(nbproject);
    if (pch > 0) printf("    removed %d stale PCH\n", pch);
    return 0;
}

static int scan_projects(const char *build_dir)
{
    DIR *d = opendir(build_dir);
    if (!d) return -1;

    struct dirent *entry;
    int total = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char proj_dir[MAX_PATH];
        snprintf(proj_dir, sizeof(proj_dir), "%s/%s", build_dir, entry->d_name);

        struct stat st;
        if (stat(proj_dir, &st) != 0 || !S_ISDIR(st.st_mode)) continue;

        char nbproject[MAX_PATH];
        snprintf(nbproject, sizeof(nbproject), "%s/nbproject", proj_dir);

        if (stat(nbproject, &st) == 0 && S_ISDIR(st.st_mode)) {
            process_nbproject(nbproject, &total);
        }
    }
    closedir(d);
    return total;
}

int main(int argc, char **argv)
{
    const char *titan_dir;
    if (argc > 1)
        titan_dir = argv[1];
    else
        titan_dir = getenv("TITAN_DIR");
    if (!titan_dir)
        titan_dir = ".";

    char build_dir[MAX_PATH];
    snprintf(build_dir, sizeof(build_dir), "%s/Projects/_Build_", titan_dir);

    struct stat st;
    if (stat(build_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: cannot find Projects/_Build_ under %s\n", titan_dir);
        return 1;
    }

    printf("Titan Linux Hotfix\n");
    printf("Scanning: %s\n", build_dir);

    int total = scan_projects(build_dir);

    if (total < 0) {
        fprintf(stderr, "Error: cannot scan %s\n", build_dir);
        return 1;
    }

    printf("\nDone. %d file(s) patched across all projects.\n", total);
    return 0;
}
