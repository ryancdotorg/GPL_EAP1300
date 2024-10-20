#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/sysinfo.h>
//#include "knlintf.h"
#include <string.h>
#include "command_list.h"

void command_list_init_node(struct command_list_st *arg, char* command)
{
    memset(arg, 0, sizeof(struct command_list_st));
    if(command)
    {
        strncpy(arg->command, command, strlen(command));
    }
}

struct command_list_st *command_list_get_head(struct list_head *head)
{
    struct list_head *iter;
    static struct command_list_st *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct command_list_st, list_member);
        return objPtr;
    }

    return 0;
}


void command_list_add_node(struct command_list_st *arg, struct list_head *head)
{
    struct command_list_st *fooPtr = (struct command_list_st *)malloc(sizeof(struct command_list_st));
    assert(fooPtr != NULL);
    memcpy(fooPtr, arg, sizeof(struct command_list_st));
    INIT_LIST_HEAD(&fooPtr->list_member);
    list_add_tail(&fooPtr->list_member, head);
}

int command_list_delete(char *command, struct list_head *head)
{
    struct list_head *iter;
    struct command_list_st *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct command_list_st, list_member);
        if(!strncmp(objPtr->command, command, strlen(command))) {
            list_del(&objPtr->list_member);
            free(objPtr);
            return 1;
        }
    }

    return 0;
}

void command_list_delete_all(struct list_head *head)
{
    struct list_head *iter;
    struct command_list_st *objPtr;

redo:
    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct command_list_st, list_member);
        list_del(&objPtr->list_member);
        free(objPtr);
        goto redo;
    }
}

void command_list_display(struct list_head *head)
{
    struct list_head *iter;
    struct command_list_st *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct command_list_st, list_member);
        printf("command:%s\n", objPtr->command);
    }
}


