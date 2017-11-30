#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<math.h>
#include <semaphore.h>
#include<string.h>
#include <time.h>

sem_t smphr;
int number = 0;

int cmp(const void *a, const void *b) {
  return *(int*)a - *(int*)b;
}

int min(int a, int b){
  return a < b ? a : b;
}

int len(int a, int b){
  if (a%b == 0){
    return a/b;
  }else{
    return a/b + 1;
  }
}

void merge(int l, int r1, int* arr, int* buf, int N, int a){
  int curL, curR, curB;
  int r = min(N,r1);
  int mid = (l+r1)/2;
  int len = r-l;
  if (r1 - l == a){
    qsort(arr+l,len,sizeof(int), cmp);
    return;
  }
  if (mid < N){
    curL = l;
    curB = l;
    curR = mid;
    while(curL < mid && curR < r){
      if (arr[curL]<arr[curR]){
        buf[curB++] = arr[curL++];
      }else{
        buf[curB++] = arr[curR++];
      }
    }
    while(curL < mid){
      buf[curB++] = arr[curL++];
    }
    while(curR < r){
      buf[curB++] = arr[curR++];
    }
    memcpy(arr+l, buf+l, len*sizeof(int));
  }
}

typedef struct Args{
  int *arr;
  int *buf;
  int *nums_iter;
  int num_threads;
  int N;
  int a;
} Args;

void* threadFunc(void* arguments){
  Args* arg = (Args*)arguments;
  int* nums_iter = arg->nums_iter;
  int num;
  sem_wait(&smphr);
  num = number++;
  sem_post(&smphr);
  int i = 0;
  int j = 0;
  int h = (int)ceil(log2(len(arg->N,arg->a))) + 1;
  int curA = arg->a;
  for(i = 0; i < h; ++i){
    int length = len(arg->N,curA);
    for(j = num; j < length; j+=arg->num_threads){
      merge(j*curA, (j+1)*curA, arg->arr, arg->buf, arg->N, arg->a);
      sem_wait(&smphr);
      ++nums_iter[i];
      sem_post(&smphr);
    }
    while (nums_iter[i] < length) {}
    curA *= 2;
  }
  return NULL;
}

int main(int argc, char* argv[]){
  if (argc < 4 || argc > 4){
    printf("wrong number og agruments\n");
    return 1;
  }
  FILE* fin;
  FILE* fout;
  FILE* stats;
  sem_init(&smphr, 0, 1);
  int a, N, h, i, num_threads, rc;
  int* arr;
  int* buffer;
  int* nums_iter;
  struct timespec begin,end;

  N = atoi(argv[1]);
  a = atoi(argv[2]);
  num_threads = atoi(argv[3]);
  pthread_t pthrs[num_threads];

  if ((fin = fopen("data.txt", "r")) == NULL){
    printf("Cannot open data.txt for reading\n");
    return 1;
  };
  arr = (int*)malloc(N*sizeof(int));
  buffer = (int*)malloc(N*sizeof(int));
  h = (int)ceil(log2(len(N,a))) + 1;
  nums_iter = (int*)calloc(h, sizeof(int));

  for (i = 0; i < N; ++i){
    fscanf(fin, "%d", arr + i);
  }
  fclose(fin);

  clock_gettime(CLOCK_REALTIME, &begin);

  for(i = 0; i < num_threads; ++i){
    Args arguments = {
      .a = a,
      .arr = arr,
      .nums_iter = nums_iter,
      .buf = buffer,
      .num_threads = num_threads,
      .N = N
    };
    rc = pthread_create(&pthrs[i], NULL, threadFunc, &arguments);
  }
  for(i = 0; i < num_threads; ++i){
    rc = pthread_join(pthrs[i], NULL);
  }

  clock_gettime(CLOCK_REALTIME, &end);

  if ((stats = fopen("stats.txt", "w")) == NULL){
    printf("Cannot open stats.txt for writing\n");
    return 1;
  };
  fprintf(stats, "%lds %d %d %d\n", end.tv_sec - begin.tv_sec, N, a, num_threads);
  fclose(stats);

  if ((fout = fopen("data-out.txt", "w")) == NULL){
    printf("Cannot open data.txt for writing\n");
    return 1;
  };
  for (i = 0; i<N; ++i){
    fprintf(fout ,"%d ",arr[i]);
  }
  fclose(fout);
  sem_destroy(&smphr);
  free(nums_iter);
  free(arr);
  free(buffer);
  return 0;
}
