#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_LINE 100
#define MAX_TIME 20 
#define MAX_DEPTH 4

void PrintHelp();
int Remove(const char* path, const int max_time, const int ignoreChanges);
int RemoveRecursiv(const char* path, int depth, const int max_time, const int ignoreChanges);
int UncomittedChanges(const char* repo_dir);

int main(int argc, char** argv)
{
    // Standard Function if no options are provided
    if(argc == 1)
    {
        Remove(".", MAX_TIME, 0);
        return 0;
    } else if(strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "--Help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "?") == 0) {
        PrintHelp();
        return 0;
    }

    int opt;
    int I_flag = 0, R_flag = 0, T_flag = 0, P_flag = 0, i = 1;
    int P_index = 0, T_index = 0;
    int time = MAX_TIME;
    int wrongParam = 0;
    char* path = ".";

    while((opt = getopt(argc, argv, "IRTP")) != -1) 
    {
        switch (opt) {
            case 'I':
                I_flag = 1;
                break;
            case 'R':
                R_flag = 1;
                break;
            case 'T':
                // check if its the last argument
                if(i + 1 == argc) {
                    wrongParam = 1;
                    break;
                }
                T_flag = 1;
                T_index = i;
                break;
            case 'P':
                // check if its the last argument
                if(i + 1 == argc) {
                    wrongParam = 1;
                    break;
                }
                P_flag = 1;
                P_index = i;
                break;
            default:
                if(i != P_index + 1 && i != T_index + 1) wrongParam = 1;
                break;
        }
        i++;
    }

    if(wrongParam) {
        PrintHelp();
        return 0;
    }
    
    if(T_flag) time = atoi(argv[T_index + 1]);
    if(P_flag) path = argv[P_index + 1];
    if(R_flag)
        printf("Deleted %d Repos\n", RemoveRecursiv(path, 0, time, I_flag));
    else
        Remove(path, time, I_flag);

    return 0;
}

void PrintHelp()
{
    printf("clrdir [-R] [-I] [-T <TIME>] [-P <PATH>]\n\n");
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
                        } else if(days >= max_time && dirty){
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

int RemoveRecursiv(const char* path, int depth, const int max_time, const int ignoreChanges)
{
    if (depth > MAX_DEPTH) return 0;
    if (chdir(path) != 0) {
        printf("Directory not found: %s\n", path);
        return 0;
    }

    DIR *d = opendir(".");
    if (!d) {
        perror("opendir failed");
        return 0;
    }

    int delCount = 0;
    struct dirent *dir;
    struct stat st;
    time_t now = time(NULL);

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        if (stat(dir->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
            // Check if it's a git repo
            char git_path[512];
            snprintf(git_path, sizeof(git_path), "%s/.git", dir->d_name);

            struct stat git_st;
            if (stat(git_path, &git_st) == 0 && S_ISDIR(git_st.st_mode)) {
                double seconds = difftime(now, git_st.st_mtime);
                int days = (int)(seconds / (60 * 60 * 24));

                int dirty = UncomittedChanges(dir->d_name);

                if ((days >= max_time && !dirty) || (days >= max_time && ignoreChanges)) {
                    printf("Removing directory: %s (last git change: %d days ago)\n", dir->d_name, days);
                    char cmd[512];
                    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", dir->d_name);
                    system(cmd);
                    delCount++;
                } else if(days >= max_time && dirty){
                    printf("Not Removing directory: %s (uncommitted changes)\n", dir->d_name);
                }
            } else {
                printf("Directory: %s not a git repo. Moving to next!\n", dir->d_name);
                // Recurse into subdirectory if not a git repo
                char subdir[512];
                snprintf(subdir, sizeof(subdir), "%s", dir->d_name);
                delCount += RemoveRecursiv(subdir, depth + 1, max_time, ignoreChanges);
            }
        }
    }

    closedir(d);
    chdir(".."); // go back to parent directory
    return delCount;
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
