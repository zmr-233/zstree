#include "zstree.h"  // 假设 zstree.h 中定义了 Elem 结构体和相关操作

// -----------p_queue头文件--自定义区域-----------
/*
typedef struct proc Elem;

extern bool reverse;

bool cmp(Elem* a, Elem* b);
*/
// -----------p_queue头文件--结构声明-----------
/*
typedef bool (*__CMPF__)(Elem*, Elem*); // 比较函数指针
typedef struct pqNode pqNode;
struct pqNode {
    Elem *elem;
    pqNode *left;
    pqNode *right;
};

typedef struct p_queue {
    pqNode *root;
    bool __isorder__;
    int size;
    __CMPF__ __cmp__;
} p_queue;

p_queue* pq_create(bool isorder, __CMPF__ cmp);
void pq_clear(p_queue *pq);
void pq_destroy(p_queue *pq);
void pq_push(p_queue *pq, Elem *elem);
void pq_pop(p_queue *pq);
Elem* pq_top(p_queue *pq);
bool pq_empty(p_queue *pq);
int pq_size(p_queue *pq);

*/
// -----------实现封装区域-----------

// 递归释放节点
static void __pq_node_destroy(pqNode* node) {
    if (node) {
        __pq_node_destroy(node->left);
        __pq_node_destroy(node->right);
        free(node);
    }
}

// 辅助函数：合并两个子堆
static pqNode* __merge(pqNode* h1, pqNode* h2, __CMPF__ cmp) {
    if (!h1) return h2;
    if (!h2) return h1;

    if (cmp(h2->elem, h1->elem)) {
        pqNode* temp = h1;
        h1 = h2;
        h2 = temp;
    }

    h1->right = __merge(h1->right, h2, cmp);

    // 平衡二叉堆：确保左子树比右子树大
    pqNode* temp = h1->left;
    h1->left = h1->right;
    h1->right = temp;

    return h1;
}

p_queue* pq_create(bool isorder, __CMPF__ cmp) {
    p_queue *pq = (p_queue*)malloc(sizeof(p_queue));
    pq->root = NULL;
    pq->__isorder__ = isorder;
    pq->size = 0;
    pq->__cmp__ = cmp;
    return pq;
}

void pq_clear(p_queue *pq) {
    if (pq->__isorder__) {
        __pq_node_destroy(pq->root);
        pq->root = NULL;
    }
    pq->size = 0;
}

void pq_destroy(p_queue *pq) {
    pq_clear(pq);
    free(pq);
}

void pq_push(p_queue *pq, Elem *elem) {
    pqNode* newNode = (pqNode*)malloc(sizeof(pqNode));
    newNode->elem = elem;
    newNode->left = newNode->right = NULL;

    if (pq->__isorder__) {
        pq->root = __merge(pq->root, newNode, pq->__cmp__);
    } else {
        // 无序的情况下，把新节点插入链表头部
        newNode->right = pq->root;
        pq->root = newNode;
    }

    pq->size++;
}

void pq_pop(p_queue *pq) {
    if (pq->__isorder__ && pq->root) {
        pqNode* oldRoot = pq->root;
        pq->root = __merge(oldRoot->left, oldRoot->right, pq->__cmp__);
        free(oldRoot); //这里是不会释放Elem*所指向的对象的，因此可以放心pop后push
        pq->size--;
    } else if (!pq->__isorder__ && pq->root) {
        pqNode* oldRoot = pq->root;
        pq->root = pq->root->right;
        free(oldRoot);
        pq->size--;
    }
}

Elem* pq_top(p_queue *pq) {
    if (pq->root) {
        return pq->root->elem;
    }
    return NULL;
}

bool pq_empty(p_queue *pq) {
    return pq->size == 0;
}

int pq_size(p_queue *pq) {
    return pq->size;
}

/*单元测试：
bool reverse = false;

bool cmp(Elem* a, Elem* b) {
    return (a->pid > b->pid) ^ reverse;
}

int main() {
    p_queue* pq = pq_create(true, cmp);

    proc p1 = { .pid = 3, .name = "Process 3", .state = 0, .ppid = 1, .pgrp = 1, .num_threads = 1, .count = 1, .threads = NULL, .son = NULL };
    proc p2 = { .pid = 1, .name = "Process 1", .state = 0, .ppid = 1, .pgrp = 1, .num_threads = 1, .count = 1, .threads = NULL, .son = NULL };
    proc p3 = { .pid = 2, .name = "Process 2", .state = 0, .ppid = 1, .pgrp = 1, .num_threads = 1, .count = 1, .threads = NULL, .son = NULL };

    pq_push(pq, &p1);
    pq_push(pq, &p2);
    pq_push(pq, &p3);

    while (!pq_empty(pq)) {
        proc* top = pq_top(pq);
        printf("Top PID: %d, Name: %s\n", top->pid, top->name);
        pq_pop(pq);
    }

    pq_destroy(pq);

    return 0;
}
*/