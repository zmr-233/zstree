#ifndef __ZSTREE_H__
#define __ZSTREE_H__

#define _GNU_SOURCE

#include <getopt.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#include <wchar.h>
#include <locale.h>



#include "marco.h"
#include "define.h"

// -----------基础类型别名-----------
typedef long lg;
typedef unsigned long ulg;
typedef long long llg;
typedef unsigned long long ullg;

static inline ulg min(ulg a, ulg b) {return a < b ? a : b;}
static inline ulg max(ulg a, ulg b) {return a > b ? a : b;}

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

// -----------flag变量+命令行参数处理-----------
extern bool pidProcFlag, //只显示进程的pid
    pidThreadFlag, //只显示线程的pid
    colorFlag, //是否显示颜色
    sortFlag, //是否按照pid排序
    foldFlag, //是否折叠打印
    forceFlag, //是否强制打印
    argsFlag; //是否显示参数

//颜色类型枚举
typedef enum COLOR_TYPE{
    NoColor, //无颜色
    StartTime, //开始时间
    ProcessState, //进程状态
}COLOR_TYPE;

extern COLOR_TYPE colorType; //颜色类型

int parse_args(int argc, char *argv[]);//解析命令行参数

// -----------进程结构定义-----------

#define MAX_PORC 2048
#define MOD 659

typedef struct thread thread;
typedef struct proc proc;

struct proc{
    int pid;
    char name[256];
    char state;
    int ppid;
    int num_threads;
    /*---解析命令行参数---*/
    char** argv;
    int argc;
    ulg start_time;
    p_queue *threads;
    p_queue *subprocs;
};

//---------全局作用域定义部分-------------

extern pid_t SELF_PID; //自身pid

extern proc* root; //进程树根节点

extern struct winsize s_win; //终端窗口大小

//时间定义部分
extern ulg TICKS_PER_SECOND, //每秒滴答数
    CURRENT_TIME, //当前时间
    MIN_START_TIME, //最小启动时间
    MAX_START_TIME; //最大启动时间
    //该最小最大启动时间用于颜色渐变的线性插值计算

// -----------build_tree进程树构建-----------

proc* build_proc(int pid);

// -----------print_tree进程树打印-----------

void print_tree(proc* p);

// -----------utils函数定义-----------
void DEB_SHOW(proc* p); //调试打印
ulg strhash(char *str); //字符串哈希
void mergehash(ulg *h1, ulg h2); //加权合并哈希

// 根据start_time计算颜色 -- 可变参
int lerp(int start, int end, float t);
void COLOR_TIME(ulg start_time, const char* fmt, ...);
void COLOR_STATE(char state, const char* fmt, ...);
void COLOR_ARGS(int argc,char** args);

#endif
