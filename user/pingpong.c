#include "kernel/types.h"
#include "user/user.h"

int main(int argc,char *argv[]){
    int pipef2s[2],pipes2f[2];
    //创建管道
    pipe(pipef2s);//father->son
    pipe(pipes2f);//son->father

    if(fork()!=0){//父进程
        write(pipef2s[1],"A",1);//1.父向子发送
        char buffer;
        read(pipes2f[0],&buffer,1);//2.父等待子回复
        printf("father(pid=%d):received pong\n",getpid());
        wait(0);
    }
    else{//子进程
        char buffer;
        read(pipef2s[0],&buffer,1);//3.子收到
        printf("son(pid=%d):received ping\n",getpid());
        write(pipes2f[1],&buffer,1);//4.子回复
    }
    exit(0);
}