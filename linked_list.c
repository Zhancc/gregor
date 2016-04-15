#include "linked_list.h"

Node* Node_new(jcb* job) {
    Node *p = (Node *)malloc(sizeof(Node));
    p->job = job;
    p->next = NULL;
    p->prev = NULL;
    return p;
}

Deque* Deque_new() {
    Deque *p = (Deque *)malloc(sizeof(Deque));
    p->head_node = NULL;
    p->tail_node = NULL;
    return p;
}

void AddNodeToTail(Deque* deque, jcb* job) {
    Node* node = Node_new(job);
    if (deque->tail_node == NULL) {
        deque->head_node = node;
        deque->tail_node = node;
    } else {
        Node* prev = deque->tail_node;
        prev->next = node;
        node->prev = prev;
        deque->tail_node = node;
    }
}

Node* GetNodeFromTail(Deque* deque) {
    if (deque->tail_node == NULL) return NULL;
    Node* prev = deque->tail_node;

    if (prev->prev == NULL) {
        deque->head_node = NULL;
    } else {
        prev->prev->next = NULL;
    }
    deque->tail_node = prev->prev;

    return prev;
}

Node* GetNodeFromHead(Deque *deque) {
    if (deque->head_node == NULL) return NULL;
    Node *head = deque->head_node;

    if (head->next == NULL) {
        deque->tail_node = NULL;
    } else {
        head->next->prev = NULL;
    }
    deque->head_node = head->next;
    return head;
}

void print_list_from_head(Node* head) {
    while (head != NULL) {
        printf("%d\n", head->value);
        head = head->next;
    }
}

void print_list_from_tail(Node* tail) {
    while (tail != NULL) {
        printf("%d\n", tail->value);
        tail = tail->prev;
    }
}

bool isEmpty(Deque *deque) {
    return deque->head_node == NULL;
}

// int main(int argc, char *argv[]) {
//     Deque* deque = Deque_new();

//     // Add node to the tail
//     printf("Add 10\n");
//     AddNodeToTail(deque, 10);

//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     // Add node to the tail
//     printf("Add 20\n");
//     AddNodeToTail(deque, 20);

//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     // Add node to the tail
//     printf("Add 30\n");
//     AddNodeToTail(deque, 30);

//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from head\n");
//     GetNodeFromHead(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from head\n");
//     GetNodeFromHead(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from head\n");
//     GetNodeFromHead(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from head\n");
//     GetNodeFromHead(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     // Add node to the tail
//     printf("Add 10\n");
//     AddNodeToTail(deque, 10);

//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from head\n");
//     GetNodeFromHead(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from head\n");
//     GetNodeFromHead(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     // Add node to the tail
//     printf("Add 10\n");
//     AddNodeToTail(deque, 10);

//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     // Add node to the tail
//     printf("Add 20\n");
//     AddNodeToTail(deque, 20);

//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from tail\n");
//     GetNodeFromTail(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);

//     printf("Remove node from tail\n");
//     GetNodeFromTail(deque);
//     print_list_from_head(deque->head_node);
//     print_list_from_tail(deque->tail_node);


//     return 0;
// }
