#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
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

#define DEFAULT_TBL_SIZE            32

#define MAX(a,b)            ((a) > (b) ? (a) : (b))

/* Directory Entry: Filename and inode */
typedef struct {
    char *fileName;
} DirEntry;

/* File Table Entry: Filename, size, and time modified */
typedef struct {
    long size;
    char *fileName;
} FileEntry;

/* File Table */
FileEntry **fileTable;

/* File Table pointer and size */
int tp, tableSize;

/*
 ******************************************************************************
 *                        Structure Managment Routines
 ******************************************************************************
*/

/* Resizes a string if needed. String must be NULL or end with null-char! */
void resizeString (char **bp, int size) {
    if (*bp == NULL) {
        *bp = malloc(size * sizeof(char));
    } else if (strlen(*bp) < size) {
        *bp = realloc(*bp, size * sizeof(char));
    } else {
        return;
    }
    assert(*bp != NULL);
}

/* Configures given DirEntry to accomodate new fileName. Manages memory. */
DirEntry setDirEntry (DirEntry entry, const char *fileName) {
    if (entry.fileName == NULL) {
        entry.fileName = calloc(strlen(fileName) + 1, sizeof(char));
    } else if (strlen(fileName) > strlen(entry.fileName)) {
        entry.fileName = realloc(entry.fileName, strlen(fileName) + 1);
    }
    assert(entry.fileName != NULL);
    entry.fileName = strcpy(entry.fileName, fileName);
    return entry;
}

/* Initializes a DirEntry and allocates + sets fileName attribute. */
DirEntry newDirEntry (const char *fileName) {
    char *sp = calloc(strlen(fileName) + 1, sizeof(char));
    assert(sp != NULL);
    return (DirEntry){.fileName = strcpy(sp, fileName)};
}

/* Free's a DirEntry. Only fileName attribute should be allocated. */
void freeDirEntry (DirEntry *dp) {
    free(dp->fileName);
}

/* Allocates a FileEntry along with fileName attribute. */
FileEntry *newFileEntry (const char *fileName, long size) {
    FileEntry *fp = malloc(sizeof(FileEntry *));
    char *sp = calloc(strlen(fileName) + 1, sizeof(char));
    assert(fp != NULL && sp != NULL);
    fp->fileName = strcpy(sp, fileName);
    fp->size = size;
    return fp;
}

/* Free's a FileEntry. Both entry and attribute should be allocated. */
void freeFileEntry (FileEntry *fp) {
    free(fp->fileName);
    free(fp);
}

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
        freeFileEntry(fileTable[i]);
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

    // Reallocate the table if necessary.
    if (tp >= tableSize) {
        tableSize = MAX(DEFAULT_TBL_SIZE, tableSize * 2);
        resizeFileTable(tableSize);
    }
    
    fileTable[tp++] = newFileEntry(fileName, fileSize);
}

/*
 ******************************************************************************
 *                          Directory Scanning Routines
 ******************************************************************************
*/

/* Returns consecutive directory entires from a directory stream */
DirEntry *nextDirectoryEntry (DIR *directory) {
    struct dirent *entryBuffer;
    static DirEntry entry = (DirEntry){.fileName = NULL};

    // Write entries to buffer as long as more exist.
    while ((entryBuffer = readdir(directory)) != NULL) {

        // Ignore unused entries (inode is zero).
        if (entryBuffer->d_ino == 0) {
            continue;
        }

        entry = setDirEntry(entry, entryBuffer->d_name);
        return &entry;
    }

    free(entry.fileName);
    return NULL;
}

/* Applies function 'f' to all valid files within given directory */
void scanDirectory (const char *directoryName, void (*f)(const char *)) {
    char *pathName = NULL;
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

        // Compute new length, resize buffer if required.
        resizeString(&pathName, strlen(directoryName) + strlen(fileName) + 2);

        // Write new path to buffer. Then scan the file.
        sprintf(pathName, "%s/%s", directoryName, fileName);
        (*f)(pathName);
    }

    // Free memory.
    free(pathName);

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
