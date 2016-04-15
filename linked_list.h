#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread.h"

typedef struct node {
    jcb* job;
    struct node* next;
    struct node* prev;
} Node;

typedef struct deque {
    Node *head_node;
    Node *tail_node;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
} Deque;

Node* Node_new(int value);
Deque* Deque_new();
void AddNodeToTail(Deque* deque, jcb* job);
Node* GetNodeFromTail(Deque* deque);
Node* GetNodeFromHead(Deque *deque);
bool isEmpty(Deque *deque);

#endif
