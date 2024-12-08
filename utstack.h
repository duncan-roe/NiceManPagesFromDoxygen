/*
Copyright (c) 2018-2022, Troy D. Hanson  https://troydhanson.github.io/uthash/
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTSTACK_H
#define UTSTACK_H

#define UTSTACK_VERSION 2.3.0

/**
 * \defgroup Stack Singly-linked list stack implementation
 *
 * This file contains macros to manipulate a singly-linked list of structures
 * as a stack.
 * To use utstack, your structure must have a "next" pointer.
 * If this pointer is called something other than "next" then you need to use
 * the macros with names ending <b>2</b>.
 *
 * <h1>Example</h1>
 *
 * \verbatim
struct item {
     int id;
     struct item *next;
};

struct item *stack = NULL;

int main() {
     int count;
     struct item *tmp;
     struct item *item = malloc(sizeof *item);
     item->id = 42;
     STACK_COUNT(stack, tmp, count); assert(count == 0);
     STACK_PUSH(stack, item);
     STACK_COUNT(stack, tmp, count); assert(count == 1);
     STACK_POP(stack, item);
     free(item);
     STACK_COUNT(stack, tmp, count); assert(count == 0);
}
\endverbatim
 *
 * \manonly
.SH SYNOPSIS
.nf
\fB
#include <uthash.h>
\endmanonly
 *
 * @{
 */

/**
 * STACK_TOP - get the top item on the stack
 * \param head Stack address
 *
 * \return address of top element on stack
 */
#define STACK_TOP(head) (head)

/**
 * STACK_EMPTY - check for empty stack
 * \param head Stack address
 *
 * \return 0 iff stack empty
 */
#define STACK_EMPTY(head) (!(head))

/**
 * STACK_PUSH - add an item to a stack
 * \param head Stack address
 * \param add pointer to new item
 */
#define STACK_PUSH(head,add) \
    STACK_PUSH2(head,add,next)

/**
 * STACK_PUSH2 - add a sructure to a stack
 * \param head Stack address
 * \param add pointer to new structure
 * \param next name of "next" pointer in structure
 */
#define STACK_PUSH2(head,add,next) \
do { \
  (add)->next = (head); \
  (head) = (add); \
} while (0)

/**
 * STACK_POP - pop an item off a stack
 * \param head Stack address
 * \param result returned pointer to popped item
 */
#define STACK_POP(head,result) \
    STACK_POP2(head,result,next)

/**
 * STACK_POP2 - pop an item off a stack
 * \param head Stack address
 * \param result returned pointer to popped item
 * \param next name of "next" pointer in structure
 */
#define STACK_POP2(head,result,next) \
do { \
  (result) = (head); \
  (head) = (head)->next; \
} while (0)

/**
 * STACK_COUNT - Count the number of items in a stack
 * \param head Stack address
 * \param el Temporary pointer to a stack element for use by the macro
 * \param counter returned count of items in head
 */
#define STACK_COUNT(head,el,counter) \
    STACK_COUNT2(head,el,counter,next)

/**
 * STACK_COUNT2 - Count the number of items in a stack
 * \param head Stack address
 * \param el Temporary pointer to a stack element for use by the macro
 * \param counter returned count of items in head
 * \param next name of "next" pointer in structure
 */
#define STACK_COUNT2(head,el,counter,next) \
do { \
  (counter) = 0; \
  for ((el) = (head); el; (el) = (el)->next) { ++(counter); } \
} while (0)

/**
 * @}
 */

#endif /* UTSTACK_H */
