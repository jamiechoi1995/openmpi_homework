#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCK 1024*1024
double *data;
int double_cmp(const void *a,const void *b){
    return *(double *)a>*(double *)b?1:-1;
}
int main(int argc,char* argv[]){
    int loop;
    int i,j;
    if(argc!=3){
        printf("Usage:%s loop_times data_file\n",argv[0]);
    }
    loop=atoi(argv[1]);
    int handle;
    handle=open(argv[2],O_RDONLY);
    if(handle==-1){
        printf("Cannot Open file %s\n",argv[2]);
        return -1;
    }
    data=(double *)malloc(sizeof(double)*BLOCK*loop);
    read(handle,data,BLOCK*loop*sizeof(double));

    qsort(data,loop*BLOCK,sizeof(double),double_cmp);

    printf("data[0]=%f last data=%f\n",data[0],data[loop*BLOCK-1]);
    close(handle);
}

