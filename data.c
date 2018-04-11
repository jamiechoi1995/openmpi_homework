#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define H 0.001
#define BLOCK 1024*1024
double f(double x){
    return x-(double)H;
}
double data[BLOCK];
int main(int argc,char* argv[]){
    int start,loop;
    int i,j;
    if(argc!=4){
        printf("Usage:%s start_value loop_times data_file\n",argv[0]);
    }
    start=atoi(argv[1]);
    loop=atoi(argv[2]);
    int handle;
    printf("starte_value=%d loop=%d\n",start,loop);
    handle=open(argv[3],O_WRONLY|O_CREAT);
    if(handle==-1){
        printf("Cannot Open file %s\n",argv[3]);
        return -1;
    }
    double s=(double)start;
    for(i=0;i<loop;i++){
        for(j=0;j<BLOCK;j++){
            data[j]=s;
            s=f(s);
        }
    write(handle,data,sizeof(double)*BLOCK);
    }
    close(handle);
}


