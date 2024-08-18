#include"zstree.h"

//---------全局作用域-------------
int pids[MAX_PORC], 
npids;

proc* root;

//最小和最大时间
ulg MIN_START_TIME = __LONG_MAX__,
    MAX_START_TIME = 0;

bool reverse = true;
bool cmp(Elem* a, Elem* b) {
    return (a->pid > b->pid) ^ reverse;
}

//---------局部作用域-------------

static pid_t SELF_PID; //自身pid
static ulg TICKS_PER_SECOND; //每秒滴答数
static ulg CURRENT_TIME; //当前时间

//flag变量:
static bool pidProcFlag=false, //只显示进程的pid
    pidThreadFlag=false, //只显示线程的pid
    colorFlag=false, //是否显示颜色
    sortFlag=false; //是否按照pid排序

//类型选择
typedef enum COLOR_TYPE{
    NoColor, //无颜色
    StartTime, //开始时间
    ProcessState, //进程状态
}COLOR_TYPE;
static COLOR_TYPE colorType=NoColor;

static void __init__(){
    setlocale(LC_ALL, "");
    SELF_PID=getpid();
    TICKS_PER_SECOND = sysconf(_SC_CLK_TCK);
    CURRENT_TIME = TICKS_PER_SECOND * time(NULL);
}
static void __process_info__(){
    if (colorType == StartTime) {
        // Rainbow-like gradient bar
        printf("Time Gradient: ");
        int div = 32;
        for (int i = 0; i < div; i++) {
            float t = (float)i / (div-1);
            int r = lerp(250, 160, t);
            int g = lerp(1, 254, t);
            int b = lerp(109, 101, t);
            printf("\x1b[48;2;%d;%d;%dm ", r, g, b); // Print color block
        }
        printf(RESET "\n"); // Reset color
    } else if (colorType == ProcessState) {
        // Process state color legend with descriptions
        printf("Process State Colors:\n");
        printf("\x1b[48;2;46;213;115m R (Running) " RESET);  // Running
        printf("\x1b[48;2;112;161;255m S (Sleeping) " RESET); // Sleeping
        printf("\x1b[48;2;55;66;250m D (Disk Sleep) " RESET); // Disk sleep
        printf("\x1b[48;2;155;170;170m Z (Zombie) " RESET);   // Zombie
        printf("\x1b[48;2;255;165;2m T (Stopped) " RESET);    // Stopped
        printf("\x1b[48;2;255;99;72m t (Tracing Stop) " RESET); // Tracing stop
        printf("\x1b[48;2;14;14;24m X (Dead) " RESET);        // Dead
        printf("\n");
    }
}

static int parse_args(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"proc-pids", no_argument, NULL, 'p'},
        {"all-pids", no_argument, NULL, 'P'},
        {"version", no_argument, NULL, 'V'},
        {"color", optional_argument, NULL, 'C'},
        {"numeric-sort", no_argument, NULL, 'n'},
        {0, 0, 0, 0}
    };
    const char *shortopts = "hpPVC::n";
    int opt = 0, index = 0;

    while ((opt = getopt_long_only(argc, argv, shortopts, long_options, &index)) != -1) {
        switch (opt) {
            case 'h':
                nECHO(GREEN_BOLD, "Usage: "); nECHO(YELLOW, "progname ");
                nECHO(RED_BOLD, "[-hpPVnC] [PID]\n\n");

                nECHO(BOLD, "Display process information.\n\n");
                nECHO(GREEN_BOLD, "  -h, --help          "); nECHO(GREY, "display this help and exit\n");
                nECHO(GREEN_BOLD, "  -p, --proc-pids     "); nECHO(GREY, "show process IDs only\n");
                nECHO(GREEN_BOLD, "  -P, --all-pids      "); nECHO(GREY, "show both process and thread IDs\n");
                nECHO(GREEN_BOLD, "  -V, --version       "); nECHO(GREY, "display version information\n");
                nECHO(GREEN_BOLD, "  -C, --color[=TYPE]  "); nECHO(GREY, "colorize output based on TYPE\n");
                nECHO(GREY, "                      "); nECHO(GREY, "(optional: "); nECHO(GREEN_BOLD,"time (StartTime), state (ProcessState))\n");
                nECHO(GREEN_BOLD, "  -n, --numeric-sort  "); nECHO(GREY, "sort output by process ID\n");
                nECHO(GREEN_BOLD, "  PID                 "); nECHO(GREY, "start at this PID; default is 1 (init)\n");

                return 2;

            case 'p':
                pidProcFlag = true;
                nECHO(GREEN, "Show proc PIDs option selected only\n");
                break;

            case 'P':
                pidProcFlag = true;
                pidThreadFlag = true;
                nECHO(GREEN, "Show both proc and thread PIDs\n");
                break;

            case 'V':
                nECHO(GREEN, "Version 1.0\n");
                return 2;
                break;
            case 'C':
                colorFlag = true;
                if (optarg) {
                    if (strcmp(optarg, "=time") == 0) {
                        colorType = StartTime;
                        nECHO(GREEN, "Color option selected with argument: ");
                        nECHO(YELLOW_BOLD, "%s\n", optarg);
                    } else if (strcmp(optarg, "=state") == 0) {
                        colorType = ProcessState;
                        nECHO(GREEN, "Color option selected with argument: ");
                        nECHO(YELLOW_BOLD, "%s\n", optarg);
                    } else {
                        nECHO(RED_BOLD, "Unknown color type: %s\n", optarg);
                        return 1;
                    }
                } else if (argv[optind] && argv[optind][0] != '-') {
                    // 处理没有使用 '=' 的情况，但参数直接跟随在 `-C` 之后
                    optarg = argv[optind];
                    optind++;
                    if (strcmp(optarg, "time") == 0) {
                        colorType = StartTime;
                        nECHO(GREEN, "Color option selected with argument: ");
                        nECHO(YELLOW_BOLD, "%s\n", optarg);
                    } else if (strcmp(optarg, "state") == 0) {
                        colorType = ProcessState;
                        nECHO(GREEN, "Color option selected with argument: ");
                        nECHO(YELLOW_BOLD, "%s\n", optarg);
                    } else {
                        nECHO(RED_BOLD, "Unknown color type: %s\n", optarg);
                        return 1;
                    }
                } else {
                    nECHO(GREEN, "Color default StartTime\n");
                    colorType = StartTime;
                }
                break;

            case 'n':
                sortFlag = true;
                nECHO(GREEN, "Numeric sort option selected\n");
                break;

            case '?':
                // Handle unknown option
                nECHO(RED_BOLD, "Unknown option\n");
                return 1;

            default:
                break;
        }
    }
    return 0;
}



static void read_stat(proc* p,char* stat){
    FILE* fp=fopen(stat,"r");
    if(fp==NULL){
        ERROR("Open file %s failed",stat);
        return;
    }
    fscanf(fp,"%d %*c%s %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %*d %lu",
            &p->pid,p->name,&p->state,&p->ppid,
            &p->num_threads,&p->start_time);
    p->name[strlen(p->name)-1]='\0';
    MIN_START_TIME = min(p->start_time,MIN_START_TIME);
    MAX_START_TIME = max(p->start_time,MAX_START_TIME);
}
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
            proc* th=(proc*)malloc(sizeof(proc));
            read_stat(th,stat);
            pq_push(p->threads,th);
        }
    }
    closedir(dp);
    // assert(p->num_threads==p->threads->size);
}
static proc* build_proc(int pid);
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

static proc* build_proc(int pid){
    char task[256],stat[256],child[256];
    sprintf(stat,"/proc/%d/stat",pid);
    sprintf(task,"/proc/%d/task",pid);
    sprintf(child,"/proc/%d/task/%d/children",pid,pid);
    
    proc* p=(proc*)malloc(sizeof(proc));
    p->subprocs=pq_create(sortFlag,cmp);
    p->threads=pq_create(sortFlag,cmp);

    //1.处理stat
    read_stat(p,stat);
    //2.处理线程
    read_threads(p,task);
    //3.处理子进程
    read_children(p,child);

    return p;
}

//----------------------打印相关操作----------------------

//前缀记录
static int pref[4096];
static int npref=0;

//│ ├ ─ ┬ └ ├=> 1 2 3 4 5
static wchar_t treeChar[] = {L'\u2502',L'\u251c', L'\u2500', L'\u252c', L'\u2514'};

typedef enum TREE_CHAR {
    NONE,
    VLINE = 0x2502,    // │
    LBRANCH = 0x251C,  // ├─
    TTEE = 0x252C,     // ─┬─
    HLINE = 0x2500,    // ───
    BLCORNER = 0x2514  // └─
}TREE_CHAR;

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
}
static void __TREE_PREF__(){
    for(int i=0;i<npref-3;i++)
        if(pref[i]==VLINE) printf("│");
        else printf(" ");
}
static void print_pre(int* cnt,int *subcnt){
    if(*cnt==1 && *subcnt==1){
        __TREE_CHAR__(HLINE);
    }
    else if(*cnt==1){
        __TREE_CHAR__(TTEE);
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
static void print_tree(proc* p);
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

static void print_tree(proc* p){
    if(p==NULL) return;
    int slen;
    if(pidProcFlag) slen = snprintf(NULL, 0, "%s(%d)", p->name, p->pid);
    else slen = snprintf(NULL, 0, "%s", p->name);

    npref=npref+slen+3;
    int subproc = p->subprocs->size,subthread = p->threads->size;
    int subcnt = subproc+subthread,cnt=1;

    //1.处理当前进程
    print_proc(p);
    //2.处理线程
    print_procs(p,&cnt,&subcnt);
    //3.处理子进程
    print_threads(p,&cnt,&subcnt);

    if(subcnt==0)printf("\n");

    npref = npref - slen - 3;
}

int main(int argc, char *argv[]){
    __init__();
    if(argc==1){
        pidProcFlag=false,
        pidThreadFlag=false,
        colorFlag=true,
        sortFlag=true,
        colorType=StartTime;
    }else{
        int state = parse_args(argc, argv);
        if(state) return 0;
    }
    __process_info__();
    //IFDEF(DEB,{pidProcFlag=true,pidThreadFlag=true,colorFlag=true,sortFlag=true,colorType=StartTime;});
    //IFDEF(DEB,{pidProcFlag=false,pidThreadFlag=false,colorFlag=true,sortFlag=true,colorType=StartTime;});
    //IFDEF(DEB,{pidProcFlag=false,pidThreadFlag=false,colorFlag=true,sortFlag=true,colorType=ProcessState;});
    root = build_proc(1);
    
    //IFDEF(DEB,DEB_SHOW(root));
    //IFDEF(DEB,DEBUG("%d",root->threads->root->elem->)); 
    print_tree(root);
}
