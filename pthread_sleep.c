
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

/****************************************************************************** 
  pthread_sleep takes an integer number of seconds to pause the current thread 
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
  *****************************************************************************/
// Functions are explained in README file.

static int __cars[4] = {0, 0, 0, 0};
pthread_mutex_t inter[4], N, E, S, W, off, synch;
pthread_cond_t officer = PTHREAD_COND_INITIALIZER;

char *getTime(time_t);

struct vector
{
  time_t time;
  struct vector *next;
};
typedef struct vector stack;

stack **stackarr = NULL;

void pusht(time_t stamp, stack **arr)
{
  if (!(*arr))
  {
    *arr = (stack *)malloc(sizeof(stack));
    (*arr)->time = stamp;
    (*arr)->next = NULL;
    return;
  }
  stack *traverse = *arr;
  while (traverse->next)
  {
    traverse = traverse->next;
  }
  traverse->next = malloc(sizeof(stack));
  traverse = traverse->next;
  traverse->time = stamp;
  traverse->next = NULL;
}

time_t poopt(stack **arr)
{
  stack *traverse = (*arr)->next;
  time_t arrival = (*arr)->time;
  free(*arr);
  *arr = traverse;
  return arrival;
}

time_t waited_for(stack *arr)
{
  if (!arr)
  {
    return 0;
  }

  time_t now = time(NULL);
  return now - arr->time;
}

int most_waited(stack **arrofarrs)
{
  time_t max = 0;
  int saved = 0;
  for (int i = 3; i > -1; i--)
  {
    if (max <= waited_for(arrofarrs[i]))
    {
      max = waited_for(arrofarrs[i]);
      saved = i;
    }
  }
  return saved;
}
/*
1-1-1-1 -- 0
0111 --- 1
0011 -- 2
0001 -- 3
0000 -- 4
*/
int pthread_sleep(int seconds)
{
  pthread_mutex_t mutex;
  pthread_cond_t conditionvar;
  struct timespec timetoexpire;
  if (pthread_mutex_init(&mutex, NULL))
  {
    return -1;
  }
  if (pthread_cond_init(&conditionvar, NULL))
  {
    return -1;
  }
  struct timeval tp;
  //When to expire is an absolute time, so get the current time and add //it to our delay time
  gettimeofday(&tp, NULL);
  timetoexpire.tv_sec = tp.tv_sec + seconds;
  timetoexpire.tv_nsec = tp.tv_usec * 1000;

  pthread_mutex_lock(&mutex);
  int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&conditionvar);

  //Upon successful completion, a value of zero shall be returned
  return res;
}

int most_in_line()
{
  if (__cars[0] >= __cars[1] && __cars[0] >= __cars[2] && __cars[0] >= __cars[3])
  {
    return 0;
  }
  else if (__cars[1] > __cars[0] && __cars[1] >= __cars[2] && __cars[1] >= __cars[3])
  {
    return 1;
  }
  else if (__cars[2] > __cars[0] && __cars[2] > __cars[1] && __cars[2] >= __cars[3])
  {
    return 2;
  }
  else if (__cars[3] > __cars[0] && __cars[3] > __cars[1] && __cars[3] > __cars[2])
  {
    return 3;
  }
}

int max()
{
  int max = 0;
  if (__cars[3] > max)
  {
    max = __cars[3];
  }
  if (__cars[2] > max)
  {
    max = __cars[2];
  }
  if (__cars[1] > max)
  {
    max = __cars[1];
  }
  if (__cars[0] > max)
  {
    max = __cars[0];
  }
  return max;
}

void *traffic()
{
  FILE *cars, cop;
  FILE *offcr;
  int last_passed = -1;
  cars = fopen("cars.log", "w");
  offcr = fopen("officer.log", "w");
  fputs("carID\tDirection\tArrival-Time\tCross-Time\tWait-Time\n", cars);
  fputs("Time\tEvent\n",offcr);
  int count = 1;
  while (1)
  {
    pthread_mutex_lock(&synch);
    pthread_mutex_lock(&N);
    pthread_mutex_lock(&S);
    pthread_mutex_lock(&W);
    pthread_mutex_lock(&E);

    if (!(__cars[0] + __cars[1] + __cars[2] + __cars[3]))
    {
      pthread_mutex_lock(&off);

      pthread_mutex_unlock(&N);
      pthread_mutex_unlock(&S);
      pthread_mutex_unlock(&W);
      pthread_mutex_unlock(&E);
      time_t now = time(0);
      fprintf(offcr,"%s\t\t%s\n", getTime(now), "Cell Phone");
      pthread_cond_wait(&officer, &off);
      time_t now2 = time(0);
      fprintf(offcr,"%s\t\t%s\n", getTime(now2), "Honk");
      pthread_mutex_unlock(&off);
      pthread_sleep(3);
      continue;
    }

    pthread_mutex_unlock(&N);
    pthread_mutex_unlock(&S);
    pthread_mutex_unlock(&W);
    pthread_mutex_unlock(&E);
    time_t wait_amount, arrival;
    if (waited_for(stackarr[most_waited(stackarr)]) >= 20)
    {
      int most_wait = most_waited(stackarr);
      pthread_mutex_unlock(&inter[most_wait]);
      last_passed = most_wait;
      wait_amount = waited_for(stackarr[most_wait]);
      arrival = poopt(&stackarr[most_wait]);
    }
    else if (last_passed == -1)
    {
      pthread_mutex_unlock(&inter[0]);
      last_passed = 0;
      wait_amount = waited_for(stackarr[0]);
      arrival = poopt(&stackarr[0]);
    }
    else if (5 > max() && __cars[last_passed])
    {
      pthread_mutex_unlock(&inter[last_passed]);
      wait_amount = waited_for(stackarr[last_passed]);
      arrival = poopt(&stackarr[last_passed]);
    }
    else
    {
      last_passed = most_in_line();
      pthread_mutex_unlock(&inter[most_in_line()]);
      wait_amount = waited_for(stackarr[last_passed]);
      arrival = poopt(&stackarr[last_passed]);
    }
    char direction[2];
    if(last_passed == 0){
      strcpy(direction,"N"); 
    }
    else if(last_passed == 1){
      strcpy(direction,"E"); 
    }
    else if(last_passed == 2){
      strcpy(direction,"S"); 
    }
    else{
      strcpy(direction,"W"); 
    }
    fprintf(cars, "%d\t\t\t%s\t\t\t%s\t%s\t%ld\n", count, direction, getTime(arrival), getTime(arrival + wait_amount), wait_amount);
    count++;
    pthread_sleep(1);
  }
  pthread_exit(NULL);
}

void new_carsN()
{
  printf("+N\n");
  pthread_mutex_lock(&N);
  __cars[0]++;
  pthread_mutex_unlock(&N);
}

void left_carsN()
{
  pthread_mutex_lock(&inter[0]);
  printf("-N\n");
  pthread_mutex_lock(&N);
  __cars[0]--;
  pthread_mutex_unlock(&N);
}

void new_carsS()
{
  printf("+S\n");
  pthread_mutex_lock(&S);
  __cars[2]++;
  pthread_mutex_unlock(&S);
}

void left_carsS()
{
  pthread_mutex_lock(&inter[2]);
  printf("-S\n");
  pthread_mutex_lock(&S);
  __cars[2]--;
  pthread_mutex_unlock(&S);
}

void new_carsW()
{
  printf("+W\n");
  pthread_mutex_lock(&W);
  __cars[3]++;
  pthread_mutex_unlock(&W);
}

void left_carsW()
{
  pthread_mutex_lock(&inter[3]);
  printf("-W\n");
  pthread_mutex_lock(&W);
  __cars[3]--;
  pthread_mutex_unlock(&W);
}
void new_carsE()
{
  printf("+E\n");
  pthread_mutex_lock(&E);
  __cars[1]++;
  pthread_mutex_unlock(&E);
}

void left_carsE()
{
  pthread_mutex_lock(&inter[1]);
  printf("-E\n");
  pthread_mutex_lock(&E);
  __cars[1]--;
  pthread_mutex_unlock(&E);
}

void *car_N(void *vargp)
{
  new_carsN();
  left_carsN();
  pthread_exit(NULL);
}

void *car_S(void *vargp)
{
  new_carsS();
  left_carsS();
  pthread_exit(NULL);
}

void *car_W(void *vargp)
{
  new_carsW();
  left_carsW();
  pthread_exit(NULL);
}

void *car_E(void *vargp)
{
  new_carsE();
  left_carsE();
  pthread_exit(NULL);
}
char *editTime(int given)
{
  char *sTime = (char *)malloc(sizeof(char) * 2);
  if (given >= 10)
    sprintf(sTime, "%d", given);
  else
    sprintf(sTime, "0%d", given);
  return sTime;
}
char *getTime(time_t current_time)
{
  struct tm *ltime = localtime(&current_time);
  char *wanted = (char *)malloc(sizeof(char) * 8);
  int hour = ltime->tm_hour;
  int min = ltime->tm_min;
  int sec = ltime->tm_sec;
  sprintf(wanted, "%s:%s:%s", editTime(hour), editTime(min), editTime(sec));
  return wanted;
}

int main(int argc, char *argv[])
{
  srand(time(NULL));
  int check = atof(argv[1]) * 100;
  int tt = atoi(argv[3]);
  if (pthread_mutex_init(&N, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&S, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&W, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&E, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&inter[0], NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&inter[1], NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&inter[2], NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(&inter[3], NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }

  if (pthread_mutex_init(&off, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }

  if (pthread_mutex_init(&synch, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  pthread_mutex_lock(&inter[0]);
  pthread_mutex_lock(&inter[1]);
  pthread_mutex_lock(&inter[2]);
  pthread_mutex_lock(&inter[3]);
  pthread_mutex_lock(&synch);
  pthread_t thread_id, offid;
  time_t current_time;
  char *c_time_string;
  stackarr = (stack **)malloc(sizeof(stack *) * 4);
  for (int i = 0; i < 4; i++)
  {
    stackarr[i] = NULL;
  }

  current_time = time(NULL);
  time_t first_time = current_time;
  /* Convert to local time format. */
  c_time_string = ctime(&current_time);
  pthread_create(&thread_id, NULL, car_N, NULL);
  pusht(current_time, stackarr);
  pthread_create(&thread_id, NULL, car_E, NULL);
  pusht(current_time, stackarr + 1);
  pthread_create(&thread_id, NULL, car_S, NULL);
  pusht(current_time, stackarr + 2);
  pthread_create(&thread_id, NULL, car_W, NULL);
  pusht(current_time, stackarr + 3);
  pthread_create(&offid, NULL, traffic, NULL);
  int count = 0;
  int blocked = 0;
      printf("At: %s:\n  \t%d\n%d\t\t%d\n\t%d\n",getTime(current_time), __cars[0], __cars[3], __cars[1], __cars[2]);
  while (count <= tt)
  {

    current_time = time(NULL);
    if (current_time - first_time >= 1)
    {
      first_time = time(NULL);
      int random = rand() % 100;
      if (check > random)
      {
        pthread_create(&thread_id, NULL, car_E, NULL);
        pusht(first_time, &stackarr[1]);
      }
      random = rand() % 100;
      if (check > random)
      {
        pthread_create(&thread_id, NULL, car_S, NULL);
        pusht(first_time, &stackarr[2]);
      }
      random = rand() % 100;
      if (check > random)
      {
        pthread_create(&thread_id, NULL, car_W, NULL);
        pusht(first_time, &stackarr[3]);
      }
      if (!blocked)
      {
        random = rand() % 100;
        if (check <= random)
        {
          pthread_create(&thread_id, NULL, car_N, NULL);
          pusht(first_time, &stackarr[0]);
          blocked = 0;
        }
        else
        {
          blocked = 21;
        }
      }
      else
      {
        blocked--;
        if (!blocked)
        {
          pthread_create(&thread_id, NULL, car_N, NULL);
          pusht(first_time, &stackarr[0]);
        }
      }
      count++;
      pthread_mutex_lock(&N);
      pthread_mutex_lock(&S);
      pthread_mutex_lock(&W);
      pthread_mutex_lock(&E);
      printf("At: %s:\n  \t%d\n%d\t\t%d\n\t%d\n",getTime(current_time), __cars[0], __cars[3], __cars[1], __cars[2]);
      if (__cars[0] + __cars[1] + __cars[2] + __cars[3])
      {
        pthread_mutex_lock(&off);
        pthread_cond_signal(&officer);
        pthread_mutex_unlock(&off);
      }
      pthread_mutex_unlock(&N);
      pthread_mutex_unlock(&S);
      pthread_mutex_unlock(&W);
      pthread_mutex_unlock(&E);
      pthread_mutex_unlock(&synch);
    }
  }
  for (int i = 0; i < 4; i++)
  {
    free(stackarr[i]);
  }
  free(stackarr);
  return 0;
}


