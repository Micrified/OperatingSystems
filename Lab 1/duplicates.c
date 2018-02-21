#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <dirent.h>

/*
 ******************************************************************************
 *                                   Ex.3                                     *
 * Author(s):   Barnabas Busa, Joe Jones, Charles Randolph.                   *
 ******************************************************************************
*/ 

/*
 ******************************************************************************
 *                        Symbolic Constants & Structures
 ******************************************************************************
*/

#define MAX_NAME                    255
#define DEFAULT_TBL_SIZE            32

#define MAX(a,b)            ((a) > (b) ? (a) : (b))

/* Directory Entry: Filename and inode */
typedef struct {
    long index;
    char fileName[MAX_NAME + 1];
} DirEntry;

/* File Table Entry: Filename, size, and time modified */
typedef struct {
    long size;
    char fileName[MAX_NAME + 1];
} FileEntry;

/* File Table */
FileEntry **fileTable;

/* File Table pointer and size */
int tp, tableSize;

/*
 ******************************************************************************
 *                                File Routines
 ******************************************************************************
*/

/* Opens file and returns file-descriptor. Exits program if error */
int openFile (const char *fileName) {
    int fd;
    if ((fd = open(fileName, O_RDONLY, 0)) == -1) {
        fprintf(stderr, "Error: Couldn't open file \"%s\"!\n", fileName);
        exit(EXIT_FAILURE);
    }
    return fd;
}

/* Returns nonzero if a difference exists between two given files */
int diff (int fd1, int fd2) {
    int r1, r2;
    char c1, c2;
    do {
        r1 = read(fd1, &c1, 1);
        r2 = read(fd2, &c2, 1);
    } while (r1 == 1 && r2 == 1 && c1 == c2);
    return c1 != c2;
}

/*
 ******************************************************************************
 *                             File Table Routines
 ******************************************************************************
*/

/* Reallocates the file table to the given size. Initializes if required */
void resizeFileTable (int toSize) {
    if (fileTable == NULL) {
        fileTable = calloc(toSize, sizeof(FileEntry *));
    } else {
        fileTable = realloc(fileTable, toSize * sizeof(FileEntry *));
    }
    if (fileTable == NULL) {
        fprintf(stderr, "Error: Couldn't allocate/reallocate the FileTable!\n");
        exit(EXIT_FAILURE);
    }
}

/* Free's the file table */
void freeFileTable (void) {
    if (fileTable == NULL) {
        return;
    }
    for (int i = 0; i < tp; i++) {
        free(fileTable[i]);
    }
    free(fileTable);
}

/* Returns a pointer to a file if a duplicate exists in the file table */
FileEntry *duplicate (const char *fileName, long fileSize) {
    int fd1, fd2;
    for (int i = 0; i < tp; i++) {
        if (fileTable[i]->size != fileSize) {
            continue;
        }
        fd1 = openFile(fileName);
        fd2 = openFile(fileTable[i]->fileName);
        if (diff(fd1, fd2)) {
            close(fd1); close(fd2);
            continue;
        }
        close(fd1); close(fd2);
        return fileTable[i];
    }
    return NULL;
}

/* Tabulates a given file in the file table */
void tabulate (const char *fileName, long fileSize) {
    FileEntry *fileEntry;

    // Reallocate the table if necessary.
    if (tp >= tableSize) {
        tableSize = MAX(DEFAULT_TBL_SIZE, tableSize * 2);
        resizeFileTable(tableSize);
    }

    // Allocate a new entry and set attributes.
    if ((fileEntry = malloc(sizeof(FileEntry))) == NULL) {
        fprintf(stderr, "Error: Couldn't allocate a file entry!\n");
        exit(EXIT_FAILURE);
    } else {
        strcpy(fileEntry->fileName, fileName);
        fileEntry->size = fileSize;
    }

    fileTable[tp++] = fileEntry;
}

/*
 ******************************************************************************
 *                          Directory Scanning Routines
 ******************************************************************************
*/

/* Returns consecutive directory entires from a directory stream */
DirEntry *nextDirectoryEntry (DIR *directory) {
    struct dirent *entryBuffer;
    static DirEntry entry;

    // Write entries to buffer as long as more exist.
    while ((entryBuffer = readdir(directory)) != NULL) {

        // Ignore unused entries (inode is zero).
        if (entryBuffer->d_ino == 0) {
            continue;
        }

        entry.index = entryBuffer->d_ino;
        strncpy(entry.fileName, entryBuffer->d_name, MAX_NAME);
        entry.fileName[strlen(entry.fileName)] = '\0';
        return &entry;
    }
    return NULL;
}

/* Applies function 'f' to all valid files within given directory */
void scanDirectory (const char *directoryName, void (*f)(const char *)) {
    char pathName[MAX_NAME];
    DirEntry *entry;
    DIR *directory;

    // Open the directory.
    if ((directory = opendir(directoryName)) == NULL) {
        fprintf(stderr, "Error: Can't access directory %s! -Ignoring-\n", directoryName);
        return;
    }

    // Scan directory contents.
    while ((entry = nextDirectoryEntry(directory)) != NULL) {
        char *fileName = entry->fileName;

        // Ignore self, parent.
        if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
            continue;
        }

        // If path isn't too long, append new file path to buffer and scan it.
        if (strlen(directoryName) + strlen(fileName) + 2 > MAX_NAME) {
            fprintf(stderr, "Error: %s filepath too long! -Ignoring-\n", fileName);
        } else {
            sprintf(pathName, "%s/%s", directoryName, fileName);
            (*f)(pathName);
        }
    }

    // Close the directory.
    closedir(directory);
}

/*
 ******************************************************************************
 *                            File Scanning Routines
 ******************************************************************************
*/

/* Tabulates information about a file. If dir, dir is walked. */
void scanFile (const char *fileName) {
    struct stat statBuffer; // For use with stat()
    FileEntry *fileEntry;   // For extraction of file-table entries.

    // System call to obtain file info via stat.
    if (stat(fileName, &statBuffer) == -1) {
        fprintf(stderr, "Error: Can't access file %s! -Ignoring-\n", fileName);
        return;
    }

    // If directory, recursively apply scanFile to contents.
    if (S_ISDIR(statBuffer.st_mode)) {
        scanDirectory(fileName, scanFile);
    } else {
        int fileSize = statBuffer.st_size;

        // Print duplicate if file exists in table. Otherwise tabulate.
        if ((fileEntry = duplicate(fileName, fileSize)) != NULL) {
            fprintf(stdout, "%s and %s are the same file.\n", fileName, fileEntry->fileName);
        }  else {
            tabulate(fileName, fileSize);
        }
    }
}

/*
 ******************************************************************************
 *                                   Main
 ******************************************************************************
*/

int main (int argc, const char *argv[]) {
    char root[2] = ".";

    // Perform file-scanning routines.
    scanFile(root);

    // Free file table.
    freeFileTable();
}
