/*
    该process_args.c主要用于处理命令行参数 + flag变量控制
*/
#include "zstree.h"

//flag变量:
bool pidProcFlag=false, //只显示进程的pid
    pidThreadFlag=false, //只显示线程的pid
    colorFlag=false, //是否显示颜色
    sortFlag=false, //是否按照pid排序
    foldFlag=false; //是否折叠打印

/*
//颜色类型枚举
typedef enum COLOR_TYPE{
    NoColor, //无颜色
    StartTime, //开始时间
    ProcessState, //进程状态
}COLOR_TYPE;
*/
COLOR_TYPE colorType=NoColor; //颜色类型

//解析命令行参数
int parse_args(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"proc-pids", no_argument, NULL, 'p'},
        {"all-pids", no_argument, NULL, 'P'},
        {"version", no_argument, NULL, 'V'},
        {"color", optional_argument, NULL, 'C'},
        {"numeric-sort", no_argument, NULL, 'n'},
        {"fold-print", no_argument, 0, 'f'},
        {0, 0, 0, 0}
    };
    const char *shortopts = "hpPVCf::n";
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
                nECHO(RED_BOLD, "Unknown option\n");
                return 1;

            default:
                break;
        }
    }
    return 0;
}
