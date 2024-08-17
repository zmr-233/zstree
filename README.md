# zstree

`zstree` 是zmr233写的一个用于显示进程树的工具，类似于 `pstree`，但增加了更多功能，如基于颜色的输出、按进程 ID 排序以及显示线程信息等。

## 功能特性

- 显示进程树，并支持显示每个进程的线程。
- 可选择显示进程和线程的 ID。
- 支持按进程开始时间或进程状态进行颜色区分。
- 提供基于进程 ID 的排序选项。

## 编译和运行

### 编译

在终端中进入项目根目录，并运行以下命令进行编译：

```bash
make all
```

编译完成后，使用以下命令运行 `zstree`：

```bash
make run
```

### 使用说明

运行 `zstree` 时，可以通过以下命令行参数调整其行为：

```bash
Usage: progname [-hpPVnC] [PID]

  -h, --help          显示帮助信息并退出
  -p, --proc-pids     仅显示进程 ID
  -P, --all-pids      显示进程和线程 ID
  -V, --version       显示版本信息并退出
  -C, --color[=TYPE]  根据指定类型为输出着色 (time (StartTime), state (ProcessState))
  -n, --numeric-sort  按进程 ID 排序输出
  PID                 从此 PID 开始显示，默认为 1 (init 进程)
```

---

# zstree

`zstree` is a tool designed to display a process tree, similar to `pstree`, but with additional features like colorized output, sorting by process ID, and displaying thread information.

## Features

- Displays a process tree with the option to show each process's threads.
- Option to display process and thread IDs.
- Supports color-coded output based on start time or process state.
- Provides an option to sort output by process ID.

## Compilation and Execution

### Compile

Navigate to the root directory of the project and run the following command to compile:

```bash
make all
```

After compilation, you can run `zstree` using the command:

```bash
make run
```

### Usage

When running `zstree`, you can adjust its behavior using the following command-line options:

```bash
Usage: progname [-hpPVnC] [PID]

  -h, --help          Display this help message and exit
  -p, --proc-pids     Show process IDs only
  -P, --all-pids      Show both process and thread IDs
  -V, --version       Display version information and exit
  -C, --color[=TYPE]  Colorize output based on TYPE (time (StartTime), state (ProcessState))
  -n, --numeric-sort  Sort output by process ID
  PID                 Start displaying from this PID; default is 1 (init process)
```

---