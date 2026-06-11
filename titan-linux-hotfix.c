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

int main(int argc, char **argv)
{
    const char *titan_dir;
    if (argc > 1)
        titan_dir = argv[1];
    else
        titan_dir = getenv("TITAN_DIR");
    if (!titan_dir)
        titan_dir = ".";

    char nbproject[MAX_PATH];
    snprintf(nbproject, sizeof(nbproject), "%s/%s", titan_dir, "nbproject");

    struct stat st;
    if (stat(nbproject, &st) != 0 || !S_ISDIR(st.st_mode)) {
        snprintf(nbproject, sizeof(nbproject), "%s/Projects/_Build_/Application/nbproject", titan_dir);
        if (stat(nbproject, &st) != 0 || !S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: cannot find nbproject directory under %s\n", titan_dir);
            return 1;
        }
    }

    printf("Titan Linux Hotfix\n");
    printf("Scanning: %s\n\n", nbproject);

    const char *files[] = {
        "Makefile-Debug.mk",
        "Makefile-Release.mk",
        "configurations.xml",
        NULL
    };

    int total = 0;
    for (int i = 0; files[i]; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", nbproject, files[i]);
        printf("Processing: %s\n", files[i]);
        int result = replace_in_file(path);
        if (result > 0) {
            printf("  Fixed -nopie -> -no-pie\n");
            total++;
        } else if (result == 0) {
            printf("  No -nopie flags found\n");
        } else {
            printf("  Skipped (error)\n");
        }
    }

    printf("\nPCH cleanup:\n");
    int pch_count = remove_pch(nbproject);
    if (pch_count == 0)
        printf("  No stale PCH files found\n");

    printf("\nDone. %d file(s) patched.\n", total);
    return total > 0 ? 0 : 0;
}
