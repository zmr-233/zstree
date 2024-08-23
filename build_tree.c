/*
    该build_tree.c文件是用来构建进程树的核心部分
*/
#include "zstree.h"

static proc* new_proc(){
    proc* p=(proc*)malloc(sizeof(proc));
    p->subprocs=pq_create(sortFlag,cmp);
    p->threads=pq_create(sortFlag,cmp);
    return p;
}

// 读取stat文件并解析进程信息
static void read_stat(proc* p, char* stat) {
    FILE* fp = fopen(stat, "r");  // 打开stat文件
    if (fp == NULL) {
        ERROR("Open file %s failed", stat);
        return;
    }

    char readbuf[1024];
    if (fgets(readbuf, sizeof(readbuf), fp) == NULL) {  // 读取文件内容到缓冲区
        ERROR("Read file %s failed", stat);
        fclose(fp);
        return;
    }

    char *comm, *tmpptr;

    //1.先读取pid
    sscanf(readbuf, "%d", &p->pid);

    //2.查找括号内的进程名称
    if ((comm = strchr(readbuf, '(')) && (tmpptr = strrchr(comm, ')'))) {
        *tmpptr = '\0'; // 将右括号替换为字符串结束符
        strncpy(p->name, comm + 1, sizeof(p->name) - 1); // 复制进程名称，去掉括号
        p->name[sizeof(p->name) - 1] = '\0'; // 确保字符串以'\0'结尾
        tmpptr++; // 指针移动到右括号之后的位置
    } else {
        ERROR("Failed to parse process name in file %s", stat);
        fclose(fp);
        return;
    }
    // 3.继续解析剩余的字段
    sscanf(tmpptr, " %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %*d %lu",
           &p->state, &p->ppid, &p->num_threads, &p->start_time);
    
    // 更新全局的最小启动时间和最大启动时间
    MIN_START_TIME = min(p->start_time, MIN_START_TIME);
    MAX_START_TIME = max(p->start_time, MAX_START_TIME);
    
    fclose(fp); 
}

// 读取cmdline文件
static void read_cmdline(proc* p, char* cmdline) {
    if (argsFlag) {
        FILE* fp = fopen(cmdline, "r");
        if (fp == NULL) {
            ERROR("Open file %s failed", cmdline); return;
        }

        // 初始化一个动态缓冲区，使用一定的初始大小
        size_t buffer_size = 256;
        size_t cnt = 0;
        char* buf = (char*)malloc(buffer_size * sizeof(char));
        if (buf == NULL) {
            ERROR("Memory allocation failed");
            fclose(fp); return;
        }

        int ch; int argc = 0, nar = 0;
        while ((ch = fgetc(fp)) != EOF) {
            // 动态扩展缓冲区
            if (cnt >= buffer_size) {
                buffer_size *= 2;
                char* new_buf = (char*)realloc(buf, buffer_size * sizeof(char));
                if (new_buf == NULL) {
                    ERROR("Memory reallocation failed");
                    free(buf);
                    fclose(fp);
                    return;
                }
                buf = new_buf;
            }
            buf[cnt++] = (char)ch;
            if (ch == '\0') argc++;
        }
        fclose(fp);

        if (cnt == 0) {
            free(buf);
            p->argc = 0;
            p->argv = NULL;
            return;
        }

        p->argc = argc;
        p->argv = (char**)malloc(argc * sizeof(char*));
        if (p->argv == NULL) {
            ERROR("Memory allocation for argv failed");
            free(buf);
            return;
        }

        p->argv[nar++] = buf;
        for (size_t i = 0; i < cnt - 1; i++) {
            if (buf[i] == '\0') {
                p->argv[nar++] = buf + i + 1;
            }
        }
    } else {
        p->argc = 0;
        p->argv = NULL;
    }
}

// 读取线程信息
static void read_threads(proc* p,char* task){
    DIR* dp=opendir(task);
    if(dp==NULL){
        ERROR("Open file %s failed",task);
        return;
    }
    char *stat;
    struct dirent *entry;
    while((entry=readdir(dp))){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        int pid=atoi(entry->d_name);
        if(pid != p->pid){
            int needed_size = snprintf(NULL, 0, "%s/%d/stat", task, pid) + 1;
            stat = (char*)malloc(needed_size);
            if (stat != NULL) {
                snprintf(stat, needed_size, "%s/%d/stat", task, pid);
            }
            proc* th=new_proc();
            read_stat(th,stat);
            pq_push(p->threads,th);
        }
    }
    closedir(dp);
    // assert(p->num_threads==p->threads->size);
}

// 读取子进程信息
static void read_children(proc* p,char* child){
    FILE* fp=fopen(child,"r");
    if(fp==NULL){
        ERROR("Open file %s failed",child);
        return;
    }
    int pid;
    while(fscanf(fp,"%d",&pid)!=EOF){
        proc* sub=build_proc(pid);
        pq_push(p->subprocs,sub);
    }
    fclose(fp);
}

//----------------------构建核心操作----------------------
proc* build_proc(int pid){
    char task[64],stat[64],child[64],cmdline[64];
    snprintf(stat,64,"/proc/%d/stat",pid);
    snprintf(task,64,"/proc/%d/task",pid);
    snprintf(child,64,"/proc/%d/task/%d/children",pid,pid);
    snprintf(cmdline,64,"/proc/%d/cmdline",pid);
    
    proc* p=new_proc();

    //1.处理stat
    read_stat(p,stat);
    // 处理cmdline
    read_cmdline(p, cmdline);

    //2.处理线程
    read_threads(p,task);
    //3.处理子进程
    read_children(p,child);

    return p;
}