#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"
#define MAX_LEN 100 // 参数最大长度

int main(int argc, char *argv[])
{
    char *command = argv[1];
    char bf;
    char argarr[MAXARG][MAX_LEN]; // 参数数组[参数编号][参数长度] 保存每个参数的字符
    char *p[MAXARG];              // 存储命令参数的指针数组

    while (1)
    {
        int count = argc - 1;                // 当前命令行参数数量
        memset(argarr, 0, MAXARG * MAX_LEN); // 初始化

        // 复制初始参数到数组中
        for (int i = 1; i < argc; i++)
        {
            strcpy(argarr[i - 1], argv[i]);
        }

        int index = 0;   // 单个参数中字符的索引
        int flag = 0;    // 读取标记
        int read_result; // 读取的字节数

        // 保存额外参数
        while (((read_result = read(0, &bf, 1))) > 0 && bf != '\n')
        { // 读到末尾或者换行就停止
            // 当前参数读取完
            if (bf == ' ' && flag == 1)
            {
                count++; // 下一个参数编号
                // 初始化
                index = 0;
                flag = 0;
            }
            else if (bf != ' ') // 忽略空格，开始本次参数
            {
                argarr[count][index++] = bf; // 保存字符
                flag = 1;
            }
        }
        // 结束
        if (read_result <= 0)
        {
            break;
        }
        // 把参数转换为指针数组
        for (int i = 0; i < MAXARG - 1; i++)
        {
            // 指向每个参数的首字符
            p[i] = argarr[i];
        }
        p[MAXARG - 1] = 0;
        if (fork() == 0)
        { // 子进程执行命令
            exec(command, p);
            exit(0);
        }
        else
        {
            wait((int *)0);
        }
    }
    exit(0);
}