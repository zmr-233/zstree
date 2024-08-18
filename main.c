#include"zstree.h"

//---------全局作用域定义部分-------------
pid_t SELF_PID; //自身pid

proc* root; //进程树根节点

//时间定义部分
ulg TICKS_PER_SECOND, //每秒滴答数
    CURRENT_TIME, //当前时间
    MIN_START_TIME = __LONG_MAX__, //最小启动时间
    MAX_START_TIME = 0; //最大启动时间
    //该最小最大启动时间用于颜色渐变的线性插值计算

//指定pid比较模式
bool reverse = true;
bool cmp(Elem* a, Elem* b) {
    return (a->pid > b->pid) ^ reverse;
}

//---------初始化部分-------------
static void __init__(){
    setlocale(LC_ALL, ""); //设置本地化
    SELF_PID=getpid();
    TICKS_PER_SECOND = sysconf(_SC_CLK_TCK);
    CURRENT_TIME = TICKS_PER_SECOND * time(NULL);
}

//---------颜色信息备注-------------
static void __process_info__(){
    if (colorType == StartTime) {
        // 进程启动时间
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
        // 进程状态信息
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

//----------------------主函数----------------------
#ifdef DEB
int main(int argc, char *argv[]){
    __init__();
    
    pidProcFlag=false,
    pidThreadFlag=false,
    colorFlag=true,
    sortFlag=true,
    foldFlag=true,
    colorType=StartTime;

    __process_info__(); // 打印颜色信息备注
    //IFDEF(DEB,{pidProcFlag=true,pidThreadFlag=true,colorFlag=true,sortFlag=true,colorType=StartTime;});
    root = build_proc(1);
    //IFDEF(DEB,DEB_SHOW(root));
    print_tree(root);
}
#else
int main(int argc, char *argv[]){
    __init__();
    if(argc==1){
        //指定无参数运行的默认行为
        pidProcFlag=false,
        pidThreadFlag=false,
        colorFlag=true,
        sortFlag=true,
        colorType=StartTime;
    }else{
        //解析命令行参数
        int state = parse_args(argc, argv);
        if(state) return 0;
    }
    __process_info__(); // 打印颜色信息备注
    //IFDEF(DEB,{pidProcFlag=true,pidThreadFlag=true,colorFlag=true,sortFlag=true,colorType=StartTime;});
    root = build_proc(1);
    //IFDEF(DEB,DEB_SHOW(root));
    print_tree(root);
}
#endif
