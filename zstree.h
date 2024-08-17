#ifndef __ZSTREE_H__
#define __ZSTREE_H__

#include <getopt.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#include <wchar.h>
#include <locale.h>



#include "marco.h"
#include "define.h"



typedef long lg;
typedef unsigned long ulg;
typedef long long llg;
typedef unsigned long long ullg;

static inline ulg min(ulg a, ulg b) {
    return a < b ? a : b;
}

static inline ulg max(ulg a, ulg b) {
    return a > b ? a : b;
}

// -----------p_queue头文件--自定义区域-----------
typedef struct proc Elem;
extern bool reverse;

bool cmp(Elem* a, Elem* b);
// -----------p_queue头文件--结构声明-----------
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

// -----------进程结构定义-----------

#define MAX_PORC 2048
#define MOD 659

typedef struct thread thread;
typedef struct proc proc;

struct proc{
    int pid;
    char name[256];
    int state;
    int ppid;
    int num_threads;
    ulg start_time;
    p_queue *threads;
    p_queue *subprocs;
};

extern int pids[MAX_PORC];
extern int npids;
// extern ulg hsh[MOD];
extern proc* root;

//最小和最大时间
extern ulg MIN_START_TIME,
           MAX_START_TIME;

// -----------utils函数定义-----------
void DEB_SHOW(proc* p);
ulg strhash(char *str);

// 根据start_time计算颜色 -- 可变参
int lerp(int start, int end, float t);
void COLOR_TIME(ulg start_time, const char* fmt, ...);
void COLOR_STATE(char state, const char* fmt, ...);

#endif
