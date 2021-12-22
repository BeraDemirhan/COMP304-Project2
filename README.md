# COMP304-Project2
My code continues in pthread_sleep.c
All parts are working and desired log files for p = 0.4, t = 60 sec log files are added.
officer.log is empty as p=0.4 is high for the officer to play with his/her phone because cars keep coming.
To see officer.log working, try a lower p value such as p = 0.1

## Implementation
There are 6 mutex locks, 4 for directions (to control the flow of the traffic) mutex off is used for when officer plays with cellphone and sync for syncronization for the flow (used to avoid deadlock when no cars arrive and police playing with the phone. The deadlock is caused by both cars and police takes N S E W mutexes.)

The variable stackarr creates 4 queues using their given priority N > E > S > W all of which are individual LinkedLists. (for example stackarr[3] = West Linked List) 
void pusht checks if the given stack has any cars yet. If not creates one using malloc. If there are cars, adds another to the related list.
void poopt takes a car out of queue meaning it crossed the intersection and left the program or in other words the thread (which is a car) completes its execution.
time_t waited for is a function to see the waiting time for each individual car
most_waited is for the 20 sec (starvation) rule. It checks if 20 seconds is up and the right of way should be given to the car in question.

int most_im_line determines where are the most number of cars in terms of lanes and returning the corresponding value for them (explained in the beginning with stackarr variable)

int max is used for the 5 cars rule, if there are multiple lanes with 5 or 5+ cars, it helps to give the right of way to the max car-numbered lane.

void traffic is the flow and the most complicated part. It uses sync for deadlock avoidance between officer and lanes, and uses lane mutexes (N S E W) the first if statement is for the see if the officer will play with his/her phone. If so, it is recorded in officer.log and mutex locks are managed accordingly. conditional wait variable is called as requested in pdf. Lastly officer sleeps for 3 seconds after "Honk".

If there are cars, mutexes are managed accordingly, required functions are called and all arrival - departure times for the cars are recorded to be printed in cars.log. Also once no cars arrive from N, N stays empty for 20 seconds with a mutex lock then in 21st second, a car definitely arrives.
Then follows to void new_cars(N-S-W-E), void left_cars(N-S-W-E) These are for the threads to lock and unlock the mutexes 
editTime and getTime functions are quick solutions for printing readable time.
In main first args are taken (p and t) and with some ifs mutex locks are checked. If there are errors they get printed "mutex init failed". stackarr gets prepared and using random number, threads are created. Lastly stackarr is freed from the memory.
