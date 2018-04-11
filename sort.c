#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCK 1024*1024
double *data;
int double_cmp(const void *a,const void *b){
    return *(double *)a > *(double *)b ? 1 : -1;
}
int main(int argc,char* argv[]){
    int loop;
    if(argc != 3){
        printf("Usage:%s loop_times data_file\n",argv[0]);
    }
    loop = atoi(argv[1]);
    FILE *fp;
    if((fp = fopen(argv[2],"r")) == 0){
        printf("Cannot Open file %s\n",argv[2]);
        return -1;
    }
    data=(double *)malloc(sizeof(double)*BLOCK*loop);
    fread(data, sizeof(double) ,BLOCK*loop, fp);

    qsort(data, BLOCK*loop, sizeof(double), double_cmp);

    printf("data[0]=%f last data=%f\n", data[0], data[BLOCK*loop-1]);
    fclose(fp);
}

