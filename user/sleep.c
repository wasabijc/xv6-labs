#include "kernel/types.h"
#include "user/user.h"

int main(int argc,char *argv[]){
    if(argc<2){
        fprintf(2,"need ticks. Usage: sleep <ticks>\n");//0,1,2 标准输入，标准输出，标准错误输出
        exit(1);//错误退出
    }
    int ticks = atoi(argv[1]);//转换类型为int
    sleep(ticks);

    exit(0);
}