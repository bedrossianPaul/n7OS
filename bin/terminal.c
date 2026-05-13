#include "terminal.h"
#include <string.h>
#include <n7OS/keyboard.h>
#include <n7OS/proc.h>
#include <n7OS/fs.h>
#include "snake.h"

static char current_dir[FS_MAX_PATH] = "/";

#define TERMINAL_INPUT_SIZE 256
#define TERMINAL_HISTORY_SIZE 32

static char history[TERMINAL_HISTORY_SIZE][TERMINAL_INPUT_SIZE];
static int history_count = 0;

static void update_prompt(void);

static const char *builtin_commands[] = {
    "help", "clear", "echo", "shutdown", "ps", "pwd", "cd", "mkdir",
    "ls", "tree", "cat", "touch", "write", "append", "cp", "mv",
    "rm", "snake", "exit", "kill"
};

static int is_space(char c) {
    return c == ' ' || c == '\t';
}

static void push_history(const char *line) {
    if (!line || line[0] == '\0') {
        return;
    }
    if (history_count < TERMINAL_HISTORY_SIZE) {
        strncpy(history[history_count], line, TERMINAL_INPUT_SIZE - 1);
        history[history_count][TERMINAL_INPUT_SIZE - 1] = '\0';
        history_count++;
        return;
    }
    for (int i = 1; i < TERMINAL_HISTORY_SIZE; i++) {
        strcpy(history[i - 1], history[i]);
    }
    strncpy(history[TERMINAL_HISTORY_SIZE - 1], line, TERMINAL_INPUT_SIZE - 1);
    history[TERMINAL_HISTORY_SIZE - 1][TERMINAL_INPUT_SIZE - 1] = '\0';
}

static void redraw_input(const char *buf, int cursor, int previous_len) {
    int len = (int)strlen(buf);
    printf("\r");
    update_prompt();
    printf("%s", buf);
    for (int i = len; i < previous_len; i++) {
        printf(" ");
    }
    printf("\r");
    update_prompt();
    for (int i = 0; i < cursor; i++) {
        printf("%c", buf[i]);
    }
}

static int complete_builtin(const char *prefix, char *out, int out_size) {
    int matches = 0;
    char candidate[TERMINAL_INPUT_SIZE];
    candidate[0] = '\0';

    for (unsigned int i = 0; i < sizeof(builtin_commands) / sizeof(builtin_commands[0]); i++) {
        if (strncmp(builtin_commands[i], prefix, strlen(prefix)) != 0) {
            continue;
        }
        matches++;
        if (matches == 1) {
            strncpy(candidate, builtin_commands[i], sizeof(candidate) - 1);
            candidate[sizeof(candidate) - 1] = '\0';
        }
    }

    if (matches == 1) {
        strncpy(out, candidate, out_size - 1);
        out[out_size - 1] = '\0';
    }
    return matches;
}

static void autocomplete(char *line, int *cursor) {
    int start = *cursor;
    while (start > 0 && !is_space(line[start - 1])) {
        start--;
    }

    char prefix[TERMINAL_INPUT_SIZE];
    int prefix_len = *cursor - start;
    if (prefix_len < 0) prefix_len = 0;
    if (prefix_len >= (int)sizeof(prefix)) {
        prefix_len = sizeof(prefix) - 1;
    }
    memcpy(prefix, line + start, prefix_len);
    prefix[prefix_len] = '\0';

    char completion[FS_MAX_PATH];
    int matches = 0;
    if (start == 0) {
        matches = complete_builtin(prefix, completion, sizeof(completion));
        if (matches == 1) {
            int completion_len = (int)strlen(completion);
            int tail_len = (int)strlen(line + *cursor);
            if (completion_len + tail_len + 1 < TERMINAL_INPUT_SIZE) {
                memmove(line + start + completion_len, line + *cursor, tail_len + 1);
                memcpy(line + start, completion, completion_len);
                *cursor = start + completion_len;
                if (*cursor + 1 < TERMINAL_INPUT_SIZE) {
                    line[*cursor] = ' ';
                    line[*cursor + 1] = '\0';
                    (*cursor)++;
                }
            }
        }
    } else {
        matches = fs_complete(current_dir, prefix, completion, sizeof(completion));
        if (matches == 1) {
            int completion_len = (int)strlen(completion);
            int tail_len = (int)strlen(line + *cursor);
            if (completion_len + tail_len + 1 < TERMINAL_INPUT_SIZE) {
                memmove(line + start + completion_len, line + *cursor, tail_len + 1);
                memcpy(line + start, completion, completion_len);
                *cursor = start + completion_len;
            }
        }
    }
}

static int read_line(char *out, int out_size) {
    if (!out || out_size <= 0) {
        return 0;
    }

    char line[TERMINAL_INPUT_SIZE];
    char draft[TERMINAL_INPUT_SIZE];
    int len = 0;
    int cursor = 0;
    int history_pos = -1;
    int draft_saved = 0;

    line[0] = '\0';
    draft[0] = '\0';

    while (1) {
        int key = kgetch();

        if (key == KEY_RETURN) {
            line[len] = '\0';
            strncpy(out, line, out_size - 1);
            out[out_size - 1] = '\0';
            return 1;
        }

        if (key == KEY_BACKSPACE) {
            if (cursor > 0) {
                int old_len = len;
                memmove(&line[cursor - 1], &line[cursor], len - cursor + 1);
                cursor--;
                len--;
                redraw_input(line, cursor, old_len);
            }
            continue;
        }

        if (key == KEY_LEFT) {
            if (cursor > 0) {
                cursor--;
                redraw_input(line, cursor, len);
            }
            continue;
        }

        if (key == KEY_RIGHT) {
            if (cursor < len) {
                cursor++;
                redraw_input(line, cursor, len);
            }
            continue;
        }

        if (key == KEY_UP) {
            if (history_count > 0) {
                int old_len = len;
                if (history_pos == -1) {
                    if (!draft_saved) {
                        strncpy(draft, line, sizeof(draft) - 1);
                        draft[sizeof(draft) - 1] = '\0';
                        draft_saved = 1;
                    }
                    history_pos = history_count;
                }
                if (history_pos > 0) {
                    history_pos--;
                    strncpy(line, history[history_pos], sizeof(line) - 1);
                    line[sizeof(line) - 1] = '\0';
                    len = (int)strlen(line);
                    cursor = len;
                    redraw_input(line, cursor, old_len);
                }
            }
            continue;
        }

        if (key == KEY_DOWN) {
            if (history_pos != -1) {
                int old_len = len;
                if (history_pos < history_count - 1) {
                    history_pos++;
                    strncpy(line, history[history_pos], sizeof(line) - 1);
                    line[sizeof(line) - 1] = '\0';
                } else {
                    history_pos = -1;
                    if (draft_saved) {
                        strncpy(line, draft, sizeof(line) - 1);
                        line[sizeof(line) - 1] = '\0';
                    } else {
                        line[0] = '\0';
                    }
                }
                len = (int)strlen(line);
                cursor = len;
                redraw_input(line, cursor, old_len);
            }
            continue;
        }

        if (key == KEY_TAB) {
            int old_len = len;
            autocomplete(line, &cursor);
            len = (int)strlen(line);
            redraw_input(line, cursor, old_len);
            continue;
        }

        if (key >= 32 && key < 127) {
            if (len + 1 < TERMINAL_INPUT_SIZE) {
                int old_len = len;
                memmove(&line[cursor + 1], &line[cursor], len - cursor + 1);
                line[cursor] = (char)key;
                cursor++;
                len++;
                redraw_input(line, cursor, old_len);
            }
        }
    }
}

static void split_first_two_args(const char *cmd, const char *prefix, char *arg1, int arg1_size, const char **arg2) {
    const char *p = cmd + strlen(prefix);
    while (*p == ' ') p++;
    const char *space = strchr(p, ' ');
    if (space == 0) {
        arg1[0] = '\0';
        *arg2 = 0;
        return;
    }
    int len = space - p;
    if (len >= arg1_size) {
        len = arg1_size - 1;
    }
    memcpy(arg1, p, len);
    arg1[len] = '\0';
    while (*space == ' ') space++;
    *arg2 = space;
}

static void update_prompt(void) {
    printf("%s> ", current_dir);
}

void terminal() {
    printf("\f"); // Clear the screen
    sleep(500);
    init_keyboard();
    while (1) {
        printf("\n");
        update_prompt();
        char input[TERMINAL_INPUT_SIZE];
        if (read_line(input, sizeof(input))) {
            printf("\n");
            if (input[0] != '\0') {
                push_history(input);
                find_cmd(input);
            }
        }
    }
}

void find_cmd(const char *cmd) {
    // Cette fonction peut être utilisée pour implémenter la logique de recherche de commandes
    // Par exemple, vous pouvez comparer 'cmd' avec une liste de commandes disponibles et exécuter la commande correspondante
    if (strcmp(cmd, "help") == 0) {
        printf("-- Available commands --\n");
        printf("clear - Clear the terminal\n");
        printf("echo - Echo the input back to the terminal\n");
        printf("shutdown - Shutdown the computer\n");
        printf("ps - List running processes\n");
        printf("kill [PID] - Terminate a process by its PID\n");
        printf("ls [dir] - List directory contents\n");
        printf("tree [dir] - Recursive directory listing\n");
        printf("pwd - Show current directory\n");
        printf("cd [dir] - Change current directory\n");
        printf("mkdir [dir] - Create a directory\n");
        printf("cat [file] - Show file contents\n");
        printf("touch [file] - Create empty file\n");
        printf("write [file] [text] - Overwrite a file\n");
        printf("append [file] [text] - Append to a file\n");
        printf("cp [src] [dst] - Copy file or directory\n");
        printf("mv [src] [dst] - Move file or directory\n");
        printf("rm [file] - Delete a file\n");
        printf("snake - Play Snake game\n");
        printf("exit - Exit the terminal\n");
        printf("help - Show this help message\n");
        // Ajoutez d'autres commandes ici
    } else if (strcmp(cmd, "clear") == 0) {
        printf("\f"); // Clear the screen
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        printf("%s\n", cmd + 5); // Echo the text after "echo "
    } else if (strcmp(cmd, "shutdown") == 0) {
        for (int i = 0; i < 5; i++) {
            printf("\rShutting down in %d seconds...", 5 - i);
            sleep(1000); // Attendre 1 seconde
        }
        shutdown(1); // Appeler la fonction de shutdown du système
    } else if (strcmp(cmd, "ps") == 0) {
        ps(); // Appeler la fonction pour afficher les processus en cours d'exécution
    } else if (strcmp(cmd, "pwd") == 0) {
        printf("%s\n", current_dir);
    } else if (strncmp(cmd, "cd ", 3) == 0) {
        char new_dir[FS_MAX_PATH];
        if (fs_resolve_path(current_dir, cmd + 3, new_dir, sizeof(new_dir)) == 0 && fs_is_dir(new_dir)) {
            strcpy(current_dir, new_dir);
        } else {
            printf("Directory not found\n");
        }
    } else if (strncmp(cmd, "mkdir ", 6) == 0) {
        char path[FS_MAX_PATH];
        if (fs_resolve_path(current_dir, cmd + 6, path, sizeof(path)) == 0 && fs_mkdir(path) == 0) {
            printf("OK\n");
        } else {
            printf("Cannot create directory\n");
        }
    } else if (strcmp(cmd, "ls") == 0) {
        fs_ls(current_dir);
    } else if (strncmp(cmd, "ls ", 3) == 0) {
        char path[FS_MAX_PATH];
        if (fs_resolve_path(current_dir, cmd + 3, path, sizeof(path)) == 0) {
            fs_ls(path);
        } else {
            printf("Bad path\n");
        }
    } else if (strcmp(cmd, "tree") == 0) {
        fs_tree(current_dir);
    } else if (strncmp(cmd, "tree ", 5) == 0) {
        char path[FS_MAX_PATH];
        if (fs_resolve_path(current_dir, cmd + 5, path, sizeof(path)) == 0) {
            fs_tree(path);
        } else {
            printf("Bad path\n");
        }
    } else if (strncmp(cmd, "cat ", 4) == 0) {
        char path[FS_MAX_PATH];
        char buf[FS_MAX_DATA + 1];
        if (fs_resolve_path(current_dir, cmd + 4, path, sizeof(path)) == 0 && fs_read(path, buf, sizeof(buf)) >= 0) {
            printf("%s\n", buf);
        } else {
            printf("File not found\n");
        }
    } else if (strncmp(cmd, "touch ", 6) == 0) {
        char path[FS_MAX_PATH];
        if (fs_resolve_path(current_dir, cmd + 6, path, sizeof(path)) == 0 && fs_create(path) == 0) {
            printf("OK\n");
        } else {
            printf("Cannot create file\n");
        }
    } else if (strncmp(cmd, "rm ", 3) == 0) {
        char path[FS_MAX_PATH];
        if (fs_resolve_path(current_dir, cmd + 3, path, sizeof(path)) == 0 && fs_remove(path) == 0) {
            printf("OK\n");
        } else {
            printf("File not found\n");
        }
    } else if (strncmp(cmd, "cp ", 3) == 0 || strncmp(cmd, "mv ", 3) == 0) {
        char src_arg[FS_MAX_PATH];
        char src[FS_MAX_PATH];
        char dst[FS_MAX_PATH];
        const char *tail = 0;
        if (strncmp(cmd, "cp ", 3) == 0) {
            split_first_two_args(cmd, "cp ", src_arg, sizeof(src_arg), &tail);
        } else {
            split_first_two_args(cmd, "mv ", src_arg, sizeof(src_arg), &tail);
        }

        if (src_arg[0] != '\0' && tail != 0) {
            while (*tail == ' ') tail++;
            if (fs_resolve_path(current_dir, src_arg, src, sizeof(src)) == 0 && fs_resolve_path(current_dir, tail, dst, sizeof(dst)) == 0) {
                int ok = (strncmp(cmd, "cp ", 3) == 0) ? fs_copy(src, dst) : fs_move(src, dst);
                if (ok == 0) {
                    printf("OK\n");
                } else {
                    printf("Operation failed\n");
                }
            } else {
                printf("Bad path\n");
            }
        } else {
            printf("Usage: cp [src] [dst]\n");
        }
    } else if (strncmp(cmd, "append ", 7) == 0 || strncmp(cmd, "write ", 6) == 0) {
        char file_name[FS_MAX_PATH];
        const char *text = 0;
        if (strncmp(cmd, "append ", 7) == 0) {
            split_first_two_args(cmd, "append ", file_name, sizeof(file_name), &text);
        } else {
            split_first_two_args(cmd, "write ", file_name, sizeof(file_name), &text);
        }
        if (file_name[0] != '\0' && text != 0) {
            char resolved[FS_MAX_PATH];
            if (fs_resolve_path(current_dir, file_name, resolved, sizeof(resolved)) == 0) {
                if (strncmp(cmd, "append ", 7) == 0) {
                    fs_append(resolved, text);
                } else {
                    fs_write(resolved, text);
                }
                printf("OK\n");
            } else {
                printf("Bad path\n");
            }
        } else {
            printf("Usage: write [file] [text]\n");
        }
    } else if (strncmp(cmd, "kill ", 5) == 0) {
        int pid = atoi(cmd + 5); // Extraire le PID après "kill "
        kill(pid); // Appeler la fonction pour tuer le processus avec le PID donné
    } else if (strcmp(cmd, "snake") == 0) {
        snake(); // Lancer le jeu Snake
        init_keyboard();
    } else if (strcmp(cmd, "exit") == 0) {
        exit(); // Terminer le processus terminal
    }
    else {
        printf("Unknown command: %s\n", cmd);
    }
}