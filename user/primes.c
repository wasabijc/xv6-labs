#include "kernel/types.h"
#include "user/user.h"

#define MAX_NUM 35
/*
    需要关闭使用完的fd,fork把父进程的所有fd复制到子进程,
    而xv6每个进程最多容纳16个fd
*/

//递归调用筛选函数
void sieve(int pileft[2]){//pileft就是每个子进程向其父接受数字的管道
    int piright[2];//创建向该进程的子的管道，传输本次筛选的数字
    int num;//接受到的第一个数字，就是一个素数

    close(pileft[1]);//关闭向其父的管道写功能

    //从左侧读取数据,读取到末尾就会退出
    int read_res = read(pileft[0],&num,sizeof(int));
    if(read_res==0) exit(0);
    
    pipe(piright);//没有退出，继续筛选，建立管道
    if(fork()==0){//递归地产生子进程
        sieve(piright);//每个子进程都使用筛选函数
    }
    else{
        close(piright[0]);//对于该进程通向子的管道，不需要读取数据，所以关闭读功能
        printf("prime:%d \n",num);//打印读取的第一个素数

        //筛选，剩下的数如果有本素数的倍数，那么就筛除
        int thisprime = num;//以该数为本次筛选的除数
        while(read(pileft[0],&num,sizeof(int))!=0){
            if(num%thisprime!=0)//不是倍数，写入右侧管道
                write(piright[1],&num,sizeof(int));
        }
        //写完毕
        close(piright[1]);//关闭向右侧的写功能
        wait(0);//等待其子孙结束
        exit(0);//结束该进程，向上递归
    }

}

int main(int argc,char *argv[]){
    //创建管道
    int pi[2];
    pipe(pi);

    if(fork()==0){//对于第一个子进程
        sieve(pi);
    }
    else{//祖父进程

        close(pi[0]);//关闭管道的读，祖父只需要向管道写入数字
        for(int i=2;i<=MAX_NUM;i++){//从2开始向管道写入数字
            write(pi[1],&i,sizeof(int));
        }
        //写完毕
        close(pi[1]);//关闭管道的写
        wait(0);//等待子孙全部退出
    }
    exit(0);
}