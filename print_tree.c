/*
    该print_tree.c文件是用来打印进程树的核心部分
*/
#include "zstree.h"

//----------------------打印相关操作----------------------

//特殊unicode字符: │ ├ ─ ┬ └ ├=> 1 2 3 4 5
static wchar_t treeChar[] = {L'\u2502',L'\u251c', L'\u2500', L'\u252c', L'\u2514'};

//树结构枚举
typedef enum TREE_CHAR {
    NONE,
    VLINE = 0x2502,    // │
    LBRANCH = 0x251C,  // ├─
    TTEE = 0x252C,     // ─┬─
    HLINE = 0x2500,    // ───
    BLCORNER = 0x2514  // └─
}TREE_CHAR;

//打印前缀--内部使用: 前缀记录
static int pref[4096];
static int npref=0;

//打印前缀--内部使用: 树的结构构件
static void __TREE_CHAR__(TREE_CHAR c){
    switch(c){
    case VLINE:
        printf(" │ ");
        break;
    case LBRANCH:
        printf(" ├─");
        break;
    case TTEE:
        printf("─┬─");
        break;
    case HLINE:
        printf("───");
        break;
    case BLCORNER:
        printf(" └─");
        break;
    case NONE:
        printf("  ");
        break;
    }
}

//打印前缀--内部使用
static void __TREE_PREF__(){
    for(int i=0;i<npref-3;i++)
        if(pref[i]==VLINE) printf("│");
        else printf(" ");
}

//打印前缀
static void print_pre(int* cnt,int *subcnt){
    if(*cnt==1 && *subcnt==1){
        if(foldFlag){
            printf("\n");
            __TREE_PREF__();
            __TREE_CHAR__(BLCORNER);
        }else __TREE_CHAR__(HLINE);
    }
    else if(*cnt==1){
        if(foldFlag){
            printf("\n");
            __TREE_PREF__();
            __TREE_CHAR__(LBRANCH);
        }else __TREE_CHAR__(TTEE);
        pref[npref-2]=VLINE;
    }
    else if(*cnt==*subcnt){
        __TREE_PREF__();
        __TREE_CHAR__(BLCORNER);
        pref[npref-2]=NONE;
    }
    else{
        __TREE_PREF__();
        __TREE_CHAR__(LBRANCH);
    }
    (*cnt)++;
}


//1.打印当前进程信息
static void print_proc(proc* p){
    switch(colorType){
    case StartTime:
        if(pidProcFlag) COLOR_TIME(p->start_time,"%s(%d)",p->name,p->pid);
        else COLOR_TIME(p->start_time,"%s",p->name);
        break;
    case ProcessState:
        if(pidProcFlag) COLOR_STATE(p->state,"%s(%d)",p->name,p->pid);
        else COLOR_STATE(p->state,"%s",p->name);
        break;
    default:
        if(pidProcFlag) printf("%s(%d)",p->name,p->pid);
        else printf("%s",p->name);
        break;
    }

    if(argsFlag){
        if(colorFlag) COLOR_ARGS(p->argc,p->argv);
        else {
            printf(" ");
            for(int i=0;i<p->argc;i++)
                printf("%s ",p->argv[i]);
        }
    }
}

static ulg __unique_procs__(proc* p){
    ulg hsh[MOD];
    memset(hsh, 0, sizeof(hsh));
    //哈希去重
    p_queue *subprocs = pq_create(sortFlag,cmp);
    int subproc = p->subprocs->size;
    while(!pq_empty(p->subprocs)){
        proc* sub=pq_top(p->subprocs);
        pq_pop(p->subprocs);
        ulg subhash = strhash(sub->name);
        if(sub->subprocs->size == 0 && sub->threads->size == 0){
            if(hsh[subhash % MOD] == 0){
                hsh[subhash % MOD]++;
                pq_push(subprocs, sub);
            }else{
                hsh[subhash % MOD]++;
            }
        }else{
            pq_push(subprocs, sub);
        }
    }
    return subprocs->size;
}

//2.打印子进程
static void print_procs(proc* p,int* cnt,int *subcnt){
    if(p->subprocs->size==0) return;
    if(pidProcFlag){
        while(!pq_empty(p->subprocs)){
            proc* sub=pq_top(p->subprocs);
            pq_pop(p->subprocs);
            
            print_pre(cnt,subcnt);
            print_tree(sub);
        }
    }else{
        ulg hsh[MOD];
        memset(hsh, 0, sizeof(hsh));
        //哈希去重
        p_queue *subprocs = pq_create(sortFlag,cmp);
        int subproc = p->subprocs->size;
        while(!pq_empty(p->subprocs)){
            proc* sub=pq_top(p->subprocs);
            pq_pop(p->subprocs);
            ulg subhash = strhash(sub->name);
            if(sub->subprocs->size == 0 && sub->threads->size == 0){
                if(hsh[subhash % MOD] == 0){
                    hsh[subhash % MOD]++;
                    pq_push(subprocs, sub);
                }else{
                    hsh[subhash % MOD]++;
                }
            }else{
                pq_push(subprocs, sub);
            }
        }
        *subcnt = *subcnt - subproc + subprocs->size;
        //正式输出
        while(!pq_empty(subprocs)){
            proc* sub=pq_top(subprocs);
            pq_pop(subprocs);

            print_pre(cnt,subcnt);
            
            ulg subhash = strhash(sub->name);
            if(sub->subprocs->size == 0 && sub->threads->size == 0 
                && hsh[subhash % MOD] > 1){
                printf("%ld*[", hsh[subhash % MOD]);
                print_proc(sub);
                printf("]\n");
            }else{
                print_tree(sub);
            }
        }
    }
}

//3.打印线程信息
static void print_thread(proc* p){
    switch(colorType){
    case StartTime:
        if(pidThreadFlag) COLOR_TIME(p->start_time,"{%s}(%d)",p->name,p->pid);
        else COLOR_TIME(p->start_time,"{%s}",p->name);
        break;
    case ProcessState:
        if(pidThreadFlag) COLOR_STATE(p->state,"{%s}(%d)",p->name,p->pid);
        else COLOR_STATE(p->state,"{%s}",p->name);
        break;
    default:
        if(pidThreadFlag) printf("{%s}(%d)",p->name,p->pid);
        else printf("{%s}",p->name);
        break;
    }
}
static void print_threads(proc* p, int* cnt, int *subcnt) {
    if (p->threads->size == 0) return;

    if (pidThreadFlag) {
        while (!pq_empty(p->threads)) {
            proc* th = pq_top(p->threads);
            pq_pop(p->threads);
            print_pre(cnt, subcnt);
            // 无需npref+=3 and npref-=3
            print_thread(th);
            printf("\n");
        }
    } else {
        ulg hsh[MOD];
        memset(hsh, 0, sizeof(hsh));
        //哈希去重
        p_queue* threads = pq_create(sortFlag, cmp);
        int subthread = p->threads->size;
        while (!pq_empty(p->threads)) {
            proc* th = pq_top(p->threads);
            pq_pop(p->threads);
            ulg thash = strhash(th->name);
            if (hsh[thash % MOD] == 0) {
                hsh[thash % MOD]++;
                pq_push(threads, th);
            } else {
                hsh[thash % MOD]++;
            }
        }
        *subcnt = *subcnt - subthread  + threads->size;
        //正式输出
        while (!pq_empty(threads)) {
            proc* th = pq_top(threads);
            pq_pop(threads);

            print_pre(cnt, subcnt);

            ulg thash = strhash(th->name);
            if (hsh[thash % MOD] > 1) {
                printf("%ld*[", hsh[thash % MOD]);
                print_thread(th);
                printf("]\n");
            } else {
                // 无需npref+=3 and npref-=3
                print_thread(th);
                printf("\n");
            }
        }
    }
}

//----------------------打印核心操作----------------------
void print_tree(proc* p){
    if(p==NULL) return;
    int slen;
    if(pidProcFlag) slen = snprintf(NULL, 0, "%s(%d)", p->name, p->pid);
    else slen = snprintf(NULL, 0, "%s", p->name);

    //处理折叠模式
    if(foldFlag) npref=npref+4; else npref=npref+slen+3;

    int subproc = p->subprocs->size,subthread = p->threads->size;
    int subcnt = subproc+subthread,cnt=1;

    //1.处理当前进程
    print_proc(p);
    //2.处理线程
    print_procs(p,&cnt,&subcnt);
    //3.处理子进程
    print_threads(p,&cnt,&subcnt);

    if(subcnt==0)printf("\n");

    //处理折叠模式
    if(foldFlag) npref=npref - 4; else npref=npref-slen-3;
}
