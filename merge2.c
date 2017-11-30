#include<stdlib.h>
#include<stdio.h>
#include<omp.h>
#include<string.h>

int cmp(const void *a, const void *b) {
     return *(int*)a - *(int*)b;
}

int bin_search(int l, int r, int value, int* arr){
  
  if (r - l == 1){
    return arr[l]<value ? r : l;
  }
  int mid = (r + l)/2;
  return value<arr[mid] ? bin_search(l, mid, value, arr) : bin_search(mid, r, value, arr);
}

void merge_2seg_to_buf(int l1, int r1, int l2, int r2, int* arr, int* buf, int start_buf_index){
  int cur1 = l1;
  int cur2 = l2;
  int curB = start_buf_index;
  int i = 0;
  while(cur1 < r1 && cur2 < r2){
    if (arr[cur1]<arr[cur2]){
      buf[curB++] = arr[cur1++];
    }else{
      buf[curB++] = arr[cur2++];
    }
  }
  while(cur1 < r1){
    buf[curB++] = arr[cur1++];
  }
  while(cur2 < r2){
    buf[curB++] = arr[cur2++];
  }
}

void merge(int l, int r, int* arr, int* buf){
  int mid = (l + r)/2;
  int mid_left = (l + mid)/2;
  int mid_right = bin_search(mid, r, arr[mid_left], arr);
  #pragma omp parallel sections
  {
    #pragma omp section
    {
      merge_2seg_to_buf(l, mid_left, mid, mid_right, arr, buf, l);
    }
    #pragma omp section
    {
      merge_2seg_to_buf(mid_left, mid, mid_right, r, arr, buf, mid_right + mid_left - mid);
    }
  }
  memcpy(arr+l, buf+l, (r - l)*sizeof(int));
}

void merge_sort(int l, int r, int* arr, int* buf, int a){
  if (r - l <= a){
    qsort(arr+l, r - l, sizeof(int), cmp);
    return;
  }
  #pragma omp parallel sections
  {
    #pragma omp section
    {
      merge_sort(l, (l+r)/2, arr, buf, a);
    }
    #pragma omp section
    {
      merge_sort((l+r)/2, r, arr, buf, a);
    }
  }
  merge(l, r, arr, buf);
}

int main(int argc, char* argv[]){
  if (argc < 4 || argc > 4){
    printf("wrong number og agruments\n");
    return 1;
  }
  FILE* fin;
  FILE* fout;
  FILE* stats;
  int a, N, i, num_threads;
  int* arr;
  int* buffer;
  double begin, end;

  N = atoi(argv[1]);
  a = atoi(argv[2]);
  num_threads = atoi(argv[3]);
  omp_set_num_threads(num_threads);

  if ((fin = fopen("data.txt", "r")) == NULL){
    printf("Cannot open data.txt for reading\n");
    return 1;
  };
  arr = (int*)malloc(N*sizeof(int));
  buffer = (int*)malloc(N*sizeof(int));
  for (i = 0; i < N; ++i){
    fscanf(fin, "%d", arr + i);
  }
  fclose(fin);
  begin = omp_get_wtime();
  merge_sort(0, N, arr, buffer, a);
  end = omp_get_wtime();
  if ((stats = fopen("stats.txt", "w")) == NULL){
    printf("Cannot open stats.txt for writing\n");
    return 1;
  };
  fprintf(stats, "%fs %d %d %d\n", end - begin, N, a, num_threads);
  printf("%fs num=%d\n",end-begin, num_threads);
  fclose(stats);

  if ((fout = fopen("data-out.txt", "w")) == NULL){
    printf("Cannot open data.txt for writing\n");
    return 1;
  };
  
  for (i = 0; i<N; ++i){
    fprintf(fout ,"%d ",arr[i]);
  }
  
  fclose(fout);
  free(arr);
  free(buffer);
  return 0;
}
