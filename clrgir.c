#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_LINE 100
#define MAX_TIME 20 

void PrintHelp();
int Remove(const char* path, const int max_time, const int ignoreChanges);
int RemoveRecursiv(const char* path, const int max_time, const int ignoreChanges);
int UncomittedChanges(const char* repo_dir);

int main(int argc, char** argv)
{
    if(argc == 1)
        Remove(".", MAX_TIME, 0);
    else if(argc == 1 && strcmp(argv[1], "-I") == 0)
        RemoveRecursiv(".", MAX_TIME, 1);
    else if(argc == 1 && strcmp(argv[1], "-R") == 0)
        RemoveRecursiv(".", MAX_TIME, 0);
    else if(strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "--help") == 0) 
        PrintHelp();
    else if(strcmp(argv[1], "-P") == 0 && argc == 3) 
        Remove(argv[2], MAX_TIME, 0);
    else if(strcmp(argv[1], "-P") == 0 && argc == 4) 
        RemoveRecursiv(argv[2], MAX_TIME, 0);
}

void PrintHelp()
{
    printf("clrdir [-R] [-I] [-T <TIME>] [-P <PATH>]\n\n");
    printf("Clears all Gitrepos which havent been used in some time\n\n");
    printf("Options:\n");
    printf("    * -R ... Recursive\n");
    printf("    * -I ... Ignores uncommitted changes\n");
    printf("    * -T <TIME> ... Time since last edited in days (default 20)\n");
    printf("    * -P <PATH> ... Path to repos (default .)\n");
}

int Remove(const char* path, const int max_time, const int ignoreChanges) 
{
    if(chdir(path) == 0) {
        DIR *d = opendir(".");
        if(!d) {
            perror("opendir failed");
            return 1;
        }
        
        int delCount = 0;
        struct dirent *dir;
        struct stat st;
        time_t now = time(NULL);

            while ((dir = readdir(d)) != NULL) {
                // Skip "." and ".."
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                    continue;

                if (stat(dir->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
                    // Build path to .git inside the directory
                    char git_path[512];
                    snprintf(git_path, sizeof(git_path), "%s/.git", dir->d_name);

                    struct stat git_st;
                    if (stat(git_path, &git_st) == 0 && S_ISDIR(git_st.st_mode)) {
                        double seconds = difftime(now, git_st.st_mtime);
                        int days = (int)(seconds / (60 * 60 * 24));

                        int dirty = UncomittedChanges(dir->d_name);

                        if (days >= max_time && !dirty) {
                            printf("Removing directory: %s (last git change: %d days ago, clean repo)\n", dir->d_name, days);

                            // Build removal command
                            char cmd[512];
                            snprintf(cmd, sizeof(cmd), "rm -rf '%s'", dir->d_name);
                            system(cmd);
                            delCount++;
                        } else if (days >= max_time && ignoreChanges == 1) {
                            printf("Removing directory: %s (last git change: %d days ago, uncomitted changes)\n", dir->d_name, days);

                            // Build removal command
                            char cmd[512];
                            snprintf(cmd, sizeof(cmd), "rm -rf '%s'", dir->d_name);
                            system(cmd);
                            delCount++;
                        } else {
                            printf("Not Removing directory: %s (uncommitted changes)\n", dir->d_name);
                        }
                    }
                }
            }
            closedir(d);
            printf("Deleted %d Repos\n", delCount);
    } else {
        printf("Directory not found\n");
        return 1;
    }
    return 0;
}

int RemoveRecursiv(const char* path, const int max_time, const int ignoreChanges)
{

}

int UncomittedChanges(const char* repo_dir)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "git -C '%s' status --porcelain", repo_dir);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 1;
    }

    char buffer[256];
    int changes = 0;

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        changes = 1;
    }

    pclose(fp);
    return changes;
}
