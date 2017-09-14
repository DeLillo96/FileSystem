#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXNAME 255
#define MAXHEIGHT 255

typedef struct element_str {
    int type;
    char name[MAXNAME];
    struct element_str **childs;
    int nChilds;
    char *text;
} element;

typedef struct searchList_str {
	char *text;
    struct searchList_str *next;
    struct searchList_str *prev;
} searchList;

searchList *searchPaths, *probeList;

char * substr(char * string, int startIndex, int offset) {
    char * newString;

    if(startIndex < offset) {
        offset -= startIndex;
        newString = (char *)malloc(offset + 1);
        memcpy(newString, &string[startIndex], offset);
        newString[offset] = '\0';
        return newString;
    } else return NULL;
}

char * getNeedle(char * path, int reverse) {
    int i, startIndex = 1, lenPath = strlen(path);

    if(reverse == 1) {
        for(i = lenPath; i > 0; i--){
            if(path[i] == '/') {
                startIndex = i;
                return substr(path, (i + 1), lenPath);
            }
        }
    }
    for (i = startIndex; i < lenPath; i ++) {
        if(path[i] == '\0' || path[i] == '/') break;
    }
    return substr(path, startIndex, i);
}

char * getText(char * path) {
    int i, endIndex = (strlen(path) - 2);

    for (i = endIndex; i > 0; i --) {
        if(path[i] == '"') {
            break;
        }
    }
    return substr(path, (i + 1), (endIndex + 1));
}

element * getLastElement(element * fs, char * path) {
    element * probeDir;
    char * needle;
    int length , i, lenPath = strlen(path), flag = 2;

    while (1) {
        needle = getNeedle(path, 0);
        length = strlen(needle) + 1;
        lenPath -= length;
        if(lenPath > 0){
            if(fs->type == 0) {
                for(i = 0; i < fs->nChilds; i++) {
                    probeDir = fs->childs[i];
                    if(0 == strcmp(probeDir->name, needle)) {
                        free(needle);
                        needle = substr(path, length, (lenPath + length));
                        if(flag != 2) free(path);
                        path = needle;
                        fs = probeDir;
                        flag = 1;
                        break;
                    }
                }
                if(flag == 1) {
                    flag = 0;
                    continue;
                }
            }
            free(needle);
            return NULL;
        } else {
            free(needle);
            return fs;
        }
    }
}

void * create(element * fs, char * command, int type) {
    element * last = getLastElement(fs, command);
    element * new, * elem;
    char * needle;
    int i;

    if(last == NULL) {
        free(command);
        printf("no\n");
        return NULL;
    }
    needle = getNeedle(command, 1);
    for(i = 0; i < last->nChilds; i++) {
        elem = last->childs[i];
        if(elem->type == type) {
            if(0 == strcmp(elem->name, needle)) {
                free(needle);
                free(command);
                printf("no\n");
                return NULL;
            }
        }
    }
    new = (element *)malloc(sizeof(element));
    new->type = type;
    new->nChilds = 0;
    strcpy(new->name, needle);
    new->text = NULL;
    if(last->nChilds > 0) {
        last->childs = (element**)realloc(last->childs, (last->nChilds + 1)*sizeof(element *));
    } else {
        last->childs = (element**)malloc(sizeof(element *));
    }
    last->childs[ last->nChilds ] = new;
    last->nChilds++;
    free(needle);
    free(command);
    printf("ok\n");
    return NULL;
}

void * readFile(element * fs, char * command) {
    element * last = getLastElement(fs, command);
    element * el;
    char* needle  = getNeedle(command, 1);
    int i;

    if(last == NULL) {
        free(needle);
        free(command);
        printf("no\n");
        return NULL;
    }
    for(i = 0; i < last->nChilds; i++) {
        el = last->childs[i];
        if(el->type == 1) {
            if(0 == strcmp(el->name, needle)) {
                if(el->text != NULL) printf("contenuto %s\n", el->text);
                else printf("contenuto \n");
                free(needle);
                free(command);
                return NULL;
            }
        }
    }
    free(needle);
    free(command);
    printf("no\n");
    return NULL;
}

void * writeFile(element * fs, char * command) {
    element * el;
    char * needle, * text = getText(command), * path = substr(command, 0, strlen(command) - strlen(text) - 3);
    element * last = getLastElement(fs, path);
    int i;

    if(last == NULL) {
        free(path);
        free(command);
        free(text);
        printf("no\n");
        return NULL;
    }
    needle = getNeedle(path, 1);
    for(i = 0; i < last->nChilds; i++) {
        el = last->childs[i];
        if(el->type == 1) {
            if(0 == strcmp(el->name, needle)) {
                el->text = text;
                printf("ok %d\n", (int)strlen(text));
                free(path);
                free(needle);
                free(command);
                return NULL;
            }
        }
    }
    free(path);
    free(needle);
    free(command);
    free(text);
    printf("no\n");
    return NULL;
}

void * delete_r(element * fs) {
    for(int i = 0; i < fs->nChilds; i++) {
        delete_r(fs->childs[i]);
    }
    if(fs->nChilds != 0) free(fs->childs);
    if(fs->text != NULL) free(fs->text);
    free(fs);
}

void * delete(element * fs, char * command, int all) {
    element * el, * last = getLastElement(fs, command);;
    char * needle;
    int i;

    if(last == NULL) {
        free(command);
        printf("no\n");
        return NULL;
    }
    needle = getNeedle(command, 1);
    for(i = 0; i < last->nChilds; i++) {
        el = last->childs[i];
        if(0 == strcmp(el->name, needle)) {
            if(el->type == 0) {
                if(el->nChilds > 0 && all == 0) break;
                for(int j = 0; j < el->nChilds; j++) {
                    delete_r(el->childs[j]);
                }
            }
            last->nChilds--;
            if(i != last->nChilds) {
                last->childs[i] = last->childs[last->nChilds];
            }
            if(last->nChilds > 0) {
                last->childs = (element **)realloc(last->childs, last->nChilds*sizeof(element *));
            } else {
                free(last->childs);
            }
            free(el);
            free(needle);
            free(command);
            printf("ok\n");
            return NULL;
        }
    }
    free(command);
    printf("no\n");
}

int search(element * fs, char * path, char * name) {
    char newPath[MAXHEIGHT*MAXNAME];
    searchList * newList = NULL;
    element * el;
    int i, finishedFlag = 0;

    for (i = 0; i < fs->nChilds; i++) {
        strcpy(newPath, path);
        el = fs->childs[i];
        strcat(newPath, "/");
        strcat(newPath, el->name);
        if(0 == strcmp(name, el->name)){
            newList = (searchList *)malloc(sizeof(searchList));
            probeList->text = (char *)malloc(strlen(newPath) + 1);
            strcpy(probeList->text, newPath);
            probeList->next = newList;
            newList->text = NULL;
            newList->next = NULL;
            newList->prev = probeList;
            probeList = newList;
            if(finishedFlag == 0) finishedFlag++;
        }
        if(el->type == 0 && el->nChilds > 0) {
            if(0 != search(el, newPath, name) && finishedFlag == 0) finishedFlag++;
        }
    }
    return finishedFlag;
}

int main() {
    element root;
    char command[40000], * supportString;
    searchList *supportList;
    int i;
    root.type = 0;
    strcpy(root.name, "root");
    while (1) {
        fgets(command, 40000, stdin);
        if(0 == strncmp(command, "create_dir", 10)) {
            i = 11;
            while (command[i] == ' ') {
                i++;
            }
            create(&root, substr(command, i, strlen(command) - 1), 0);
            continue;
        } else {
            if(0 == strncmp(command, "create", 6)){
                i = 7;
                while (command[i] == ' ') {
                    i++;
                }
                create(&root, substr(command, i, strlen(command) - 1), 1);
                continue;
            }
        }
        if(0 == strncmp(command, "read", 4)) {
            i = 5;
            while (command[i] == ' ') {
                i++;
            }
            readFile(&root, substr(command, i, strlen(command) - 1));
            continue;
        }
        if(0 == strncmp(command, "write", 5)) {
            i = 6;
            while (command[i] == ' ') {
                i++;
            }
            writeFile(&root, substr(command, i, strlen(command) - 1));
            continue;
        }
        if(0 == strncmp(command, "delete_r", 8)) {
            i = 9;
            while (command[i] == ' ') {
                i++;
            }
            delete(&root, substr(command, i, strlen(command) - 1), 1);
            continue;
        } else {
            if(0 == strncmp(command, "delete", 6)) {
                i = 7;
                while (command[i] == ' ') {
                    i++;
                }
                delete(&root, substr(command, i, strlen(command) - 1), 0);
                continue;
            }
        }
        if(0 == strncmp(command, "find", 4)) {
            searchPaths = (searchList *)malloc(sizeof(searchList));
            searchPaths->prev = NULL;
            probeList = searchPaths;
            i = 5;
            while (command[i] == ' ') {
                i++;
            }
            if(0 != search(&root, "", substr(command, i, strlen(command) - 1))) {
                supportList = searchPaths;
                while(supportList->text != NULL) {
                    probeList = supportList->next;
                    while(probeList->text != NULL) {
                        if(strcmp(supportList->text, probeList->text) > 0) {
                            supportString = supportList->text;
                            supportList->text = probeList->text;
                            probeList->text = supportString;
                        }
                        probeList = probeList->next;
                    }
                    printf("ok %s\n", supportList->text);
                    free(supportList->text);
                    supportList = supportList->next;
                    free(supportList->prev);
                }
            } else printf("no\n");
            continue;
        }
        if(0 == strncmp(command, "exit", 4)) break;
    }
    return 0;
}
