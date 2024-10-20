#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdlib.h>

#if !defined(__bool_true_false_are_defined) && !defined(__cplusplus)
typedef int bool;
#define true 1
#define false 0
#define __bool_true_false_are_defined
#endif

#define StackItem void*

typedef struct Stack Stack;

enum{
	STACKTYPE_JSON=0,
	STACKTYPE_STR
};

typedef struct StackStruct_t
{
	int type;
	void* item;
}StackStruct;

Stack *stackCreate();
void stackDestroy(Stack *stack);
void stackClean(Stack *stack);
bool stackIsEmpty(Stack *stack);
size_t stackSize(Stack *stack);
StackItem stackTop(Stack *stack);
bool stackPush(Stack *stack, StackItem item);
StackItem stackPop(Stack *stack);

#endif
