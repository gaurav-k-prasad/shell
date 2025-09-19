#ifndef FORGETTING_DLL_STRUCTURES_H
#define FORGETTING_DLL_STRUCTURES_H

typedef struct ListNode
{
  char *command;
  struct ListNode *prev, *next;
} ListNode;

typedef struct ForgettingDoublyLinkedList
{
  int size;
  int capacity;
  ListNode *head, *tail;
} ForgettingDoublyLinkedList;

#endif