#include "zstree.h"

//注意：DEB_SHOW不打印线程，只打印进程
void DEB_SHOW(proc* p){
    if(!p){
        ERROR("Process Null");
        return;
    }

    if(p->num_threads==0) return; //该proc是线程
    DEBUG("%d { \n    name: %s\n    state: %c\n    ppid: %d\n    num_threads: %d", 
        p->pid, p->name, p->state, p->ppid, p->num_threads);
    
    //命令行参数

    //启动时间
    ECHO(YELLOW,"    start_time: %lu", p->start_time);
    
    if(p->threads){
        int threads_size = pq_size(p->threads);
        if(threads_size >= 1){
            Elem *first_thread = pq_top(p->threads);
            ECHO(YELLOW, "    threads: %s(%d)%s", 
                first_thread->name, first_thread->pid, threads_size > 1 ? "..." : "");
        }else  ECHO(YELLOW,"    threads: Null");
    } else {
        ECHO(YELLOW,"    threads: Null");
    }

    if(p->subprocs){
        int subprocs_size = pq_size(p->subprocs);
        if(subprocs_size >= 1){
            Elem *first_subproc = pq_top(p->subprocs);
            ECHO(YELLOW, "    subprocs: %s(%d)%s", 
                first_subproc->name, first_subproc->pid, subprocs_size > 1 ? "..." : "");
        }else  ECHO(YELLOW,"    subprocs: Null");
    } else {
        ECHO(YELLOW, "    subprocs: Null");
    }

    ECHO(YELLOW, "}");
}

//字符串哈希
ulg strhash(char *str) {
    ulg hash = 5381;
    int c;
    const char *p = str;
    while ((c = *p++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

//加权合并哈希
void mergehash(ulg *h1, ulg h2) {
    const ulg weight = 31;
    *h1 = (*h1 * weight) + h2;
}

//-----------线性插值渐变RGB-----------
// 线性插值函数
int lerp(int start, int end, float t) {
    return (int)(start + t * (end - start));
}

// 根据start_time计算颜色 -- 可变参
void COLOR_TIME(ulg start_time, const char* fmt, ...) {
    float t = (float)(start_time - MIN_START_TIME) / (MAX_START_TIME - MIN_START_TIME);

    int r = lerp(250, 160, t);
    int g = lerp(1, 254, t);
    int b = lerp(109, 101, t);

    va_list args;
    va_start(args, fmt);

    printf("\x1b[38;2;%d;%d;%dm", r, g, b);
    vprintf(fmt, args);
    printf("\x1b[0m");
    va_end(args);
}

//根据进程状态计算颜色
void COLOR_STATE(char state, const char* fmt, ...) {
    int r, g, b;

    // 根据进程状态分配颜色
    switch(state) {
        case 'R':  // #2ed573
            r = 46; g = 213; b = 115;
            break;
        case 'S':  // #70a1ff
            r = 112; g = 161; b = 255;
            break;
        case 'D':  // #3742fa
            r = 55; g = 66; b = 250;
            break;
        case 'Z':  // #747d8c
            r = 155; g = 170; b = 170;
            break;
        case 'T':  // #ffa502
            r = 255; g = 165; b = 2;
            break;
        case 't':  // #ff6348
            r = 255; g = 99; b = 72;
            break;
        case 'X':  // #2f3542
            r = 14; g =14 ; b = 24;
            break;
        default:   // 默认颜色，如果状态不匹配
            r = 255; g = 255; b = 255;  // 白色
            break;
    }

    va_list args;
    va_start(args, fmt);

    // 设置颜色并打印
    printf("\x1b[38;2;%d;%d;%dm", r, g, b);
    vprintf(fmt, args);
    printf("\x1b[0m");  // 重置颜色

    va_end(args);
}

//根据参数计算颜色
void COLOR_ARGS(int argc,char** args){
    int r = 129, g = 236, b = 236;  // 默认颜色为白色
    printf(" ");
    // 设置颜色并打印
    printf("\x1b[38;2;%d;%d;%dm", r, g, b);
    for(int i=0;i<argc;i++)
        printf("%s ",args[i]);
    printf("\x1b[0m");  // 重置颜色
}

/*单元测试：渐变色打印
// 打印带有渐变色的方块字符
void print_gradient_blocks(int num_blocks) {
    // 嫩绿色 (#a0fe65) 到 玫红色 (#fa016d)
    int start_r = 160, start_g = 254, start_b = 101;
    int end_r = 250, end_g = 1, end_b = 109;

    for (int i = 0; i < num_blocks; i++) {
        // 计算插值比例
        float t = (float)i / (num_blocks - 1);

        // 计算RGB值
        int r = lerp(start_r, end_r, t);
        int g = lerp(start_g, end_g, t);
        int b = lerp(start_b, end_b, t);

        // 打印方块字符
        printf("\x1b[38;2;%d;%d;%dm█\x1b[0m\n", r, g, b);
    }
    printf("\n");
}

int main() {
    int num_blocks = 120;  // 要打印的方块字符数量
    print_gradient_blocks(num_blocks);
    COLOR_STATE('R', "This is Running (R) state.");
    COLOR_STATE('S', "This is Sleeping (S) state.");
    COLOR_STATE('D', "This is Uninterruptible Sleep (D) state.");
    COLOR_STATE('Z', "This is Zombie (Z) state.");
    COLOR_STATE('T', "This is Stopped (T) state.");
    COLOR_STATE('t', "This is Traced (t) state.");
    COLOR_STATE('X', "This is Dead (X) state.");
    return 0;
}

*/