/* ********************************************************* *
 * Jared Schneider schneiderj4067@gmail.com                  *
 * MPBBS.cpp:                                                *
 *                                                           *
 *                                                           *
 *                                                           *
 * compile: "gcc MPBBS.cpp"                                  *
 *                                                           *
 *                                                           *
 * Last updated: March 30, 2023                              *
 * ********************************************************* */
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>    
#include <time.h>
#include <string.h>

// the followings are for semaphores ----- 
#include <sys/sem.h>
#include <sys/ipc.h>

// the followings are for shared memory ----
#include <sys/shm.h>
#define NUM_REPEAT    	12    // number of loops for testing 
#define READER_TIME_01	500000
#define READER_TIME_02	1000000
#define WRITER_TIME_01	2000000
#define WRITER_TIME_02	600000
// the semaphore keys
#define SEM_KEY_1       8280      
#define SEM_KEY_2       8780
#define SEM_KEY_3       9280
#define SEM_KEY_4       9580
#define SEM_KEY_5       7280
#define SEM_KEY_6       7580
// the shared memory key 
#define SHM_KEY       	8280       

#define NUM_WRITERS     2
#define NUM_READERS     4
#define NUM_CHILD       5
#define MAX_READERS     3

   // shared memory definition ----   
struct my_mem {
	   char         message[256];
       unsigned int Done_Flag[NUM_WRITERS]; //Used for while loop in readers
       unsigned int processCount;
};    

//Function Prototypes
void msleep(unsigned micro_seconds);
unsigned int uniform_rand(void);

int main (void)
{
   pid_t  process_id;
   int    pid;  
   int    i;                     // external loop counter  
   int    j;                     // internal loop counter  
   int    k = 0;                 // dumy integer  
   int    sleep_time;
   int    sem_id_1;              //MUTEX for when a writer is active
   int    sem_id_2;              //Counting Sem for concurrent Readers
   int    sem_id_3;              //MUTEX for waiting until all processes are created
   //int    sem_id_4;            //Used to be used but then realized it wasn't needed
   int    sem_id_5;              //Counting Sem that counts children terminating
   int    sem_id_6;              //Counting Sem that says no readers active
   int    ret_val;               // system-call return value    
   int    shm_id;                // the shared memory ID 
   int    shm_size;              // the size of the shared memoy


   // Semaphore control data structure ----
   union semun {
        int    val;  
        struct semid_ds  *buf;  
        ushort * arry;
   } argument; 

   struct my_mem {
	   char         message[256];
       unsigned int Done_Flag[NUM_WRITERS]; //Used for while loop in readers
       unsigned int processCount;
    };    
    struct my_mem * p_shm;        // pointer to the attached shared memory
    struct sembuf operations[1];  // Define semaphore operations
    struct sembuf mutexOperations[2];
    srand((unsigned int)time(NULL));
      
   // find the shared memory size in bytes ----
   shm_size = sizeof(struct my_mem);   
   if (shm_size <= 0)
   {  
      fprintf(stderr, "sizeof error in acquiring the shared memory size. Terminating ..\n");
      exit(0); 
   }    
   
   // create a new semaphore -----
   sem_id_1 = semget(SEM_KEY_1, 1, 0666 | IPC_CREAT); 
   if (sem_id_1 < 0)
   {
      fprintf(stderr, "Failed to create semaphore_1. Terminating ..\n"); 
      exit(0);
   }
   argument.val = 0;   // the initial value of the semaphore
   // initialzie the new semapahore by 0 
   if (semctl(sem_id_1, 0, SETVAL, argument) < 0)
   {
      fprintf(stderr, "Failed to initialize semaphore_1 by 1. Terminating ..\n"); 
      exit(0);  
   }

   sem_id_2 = semget(SEM_KEY_2, 1, 0666 | IPC_CREAT); 
   if (sem_id_2 < 0)
   {
      fprintf(stderr, "Failed to create a semaphore_2. Terminating ..\n"); 
      exit(0);
   }
    argument.val = MAX_READERS;   // the initial value of the semaphor
   // initialzie the new semapahore by Max readers
   if (semctl(sem_id_2, 0, SETVAL, argument) < 0)
   {
      fprintf(stderr, "Failed to initialize the semaphore_2 by 1. Terminating ..\n"); 
      exit(0);  
   }

   sem_id_3 = semget(SEM_KEY_3, 1, 0666 | IPC_CREAT); 
   if (sem_id_3 < 0)
   {
      fprintf(stderr, "Failed to create semaphore_3. Terminating ..\n"); 
      exit(0);
   }
    argument.val = 1;
   // initialzie the new semapahore by 1 
   if (semctl(sem_id_3, 0, SETVAL, argument) < 0)
   {
      fprintf(stderr, "Failed to initialize semaphore_3 by 1. Terminating ..\n"); 
      exit(0);  
   }

//    sem_id_4 = semget(SEM_KEY_4, 1, 0666 | IPC_CREAT); 
//    if (sem_id_4 < 0)
//    {
//       fprintf(stderr, "Failed to create semaphore_4. Terminating ..\n"); 
//       exit(0);
//    }
//     argument.val = 1;
//    // initialzie the new semapahore by 1 (zero) ----
//    if (semctl(sem_id_4, 0, SETVAL, argument) < 0)
//    {
//       fprintf(stderr, "Failed to initialize semaphore_4 by 1. Terminating ..\n"); 
//       exit(0);  
//    }

   sem_id_5 = semget(SEM_KEY_5, 1, 0666 | IPC_CREAT); 
   if (sem_id_5 < 0)
   {
      fprintf(stderr, "Failed to create a semaphore_5. Terminating ..\n"); 
      exit(0);
   }
    argument.val = NUM_CHILD;   // the initial value of the semaphor
   // initialzie the new semapahore by number of children
   if (semctl(sem_id_5, 0, SETVAL, argument) < 0)
   {
      fprintf(stderr, "Failed to initialize the semaphore_5 by 1. Terminating ..\n"); 
      exit(0);  
   }

   sem_id_6 = semget(SEM_KEY_6, 1, 0666 | IPC_CREAT); 
   if (sem_id_6 < 0)
   {
      fprintf(stderr, "Failed to create a semaphore_6. Terminating ..\n"); 
      exit(0);
   }
    argument.val = 0;   // the initial value of the semaphor
   // initialzie the new semapahore by 0
   if (semctl(sem_id_6, 0, SETVAL, argument) < 0)
   {
      fprintf(stderr, "Failed to initialize the semaphore_6 by 1. Terminating ..\n"); 
      exit(0);  
   }

   // create a shared memory ----
   shm_id = shmget(SHM_KEY, shm_size, 0666 | IPC_CREAT);         
   if (shm_id < 0) 
   {
      fprintf(stderr, "Failed to create the shared memory. Terminating ..\n");  
      exit(0);  
   } 

   // attach the new shared memory ----
   p_shm = (struct my_mem *)shmat(shm_id, NULL, 0);     
   if (p_shm == (struct my_mem*) -1)
   {
      fprintf(stderr, "Failed to attach the shared memory.  Terminating ..\n"); 
      exit(0);   
   }   

   // initialize the shared memory ----
   for(i = 0; i < NUM_WRITERS; i++){
        p_shm->Done_Flag[i]=0;
   }
   strcpy(p_shm->message,"no one has posted a message");
   p_shm->processCount = 0;

  pid = fork();
  if(pid == 0){
  	if(fork() == 0){
		if(fork() == 0){
			if(fork() == 0){
				if(fork() == 0){
					//Reader 4
                        int sleep_time;
                        if(p_shm->processCount == 5){
                            //Tells everyone to start
                            operations[0].sem_num = 0;
                            operations[0].sem_op = -1;  
                            operations[0].sem_flg = 0; 
                            ret_val = semop(sem_id_3, operations, 1);
                            if (ret_val != 0)  
                            {   
                                fprintf(stderr, "Start mutex(Readers) has failed....\a\n");   
                            }
                        }
                        else{
                            p_shm->processCount++;
                            operations[0].sem_num = 0;
                            operations[0].sem_op = 0;  
                            operations[0].sem_flg = 0; 
                            ret_val = semop(sem_id_3, operations, 1);
                            if (ret_val != 0)  
                            {   
                                fprintf(stderr, "Wait until start mutex has failed....\a\n");   
                            }
                        }
                        while(p_shm->Done_Flag[0]!= 1 || p_shm->Done_Flag[1]!=1)   
                            {
                            sleep_time = rand() % READER_TIME_01;
                            msleep(sleep_time);
                            printf("    R4 would like to read the posted message...\n");
                                //***Critical Section Begins***
                                //MUTEX: Only lets reader enter if nobody is writing
                                mutexOperations[0].sem_num = 0;
                                mutexOperations[0].sem_op = 0;   
                                mutexOperations[0].sem_flg = 0;  
                                ret_val = semop(sem_id_1, mutexOperations, 1);         
                                if (ret_val != 0)  
                                {   
                                    fprintf(stderr, "Initial Mutex R4 Failed ....\a\n");   
                                }
                                //Does opposite of the one below
                                operations[0].sem_num = 0;
                                operations[0].sem_op = 1;   
                                operations[0].sem_flg = 0;  
                                ret_val = semop(sem_id_6, operations, 1);         
                                if (ret_val != 0)  
                                {   
                                    fprintf(stderr, "R4 Sem6 Increment Failed ....\a\n");   
                                }   
                                //AVAILABLE READERS COUNT
                                operations[0].sem_num = 0;
                                operations[0].sem_op = -1;   
                                operations[0].sem_flg = 0;  
                                ret_val = semop(sem_id_2, operations, 1);         
                                if (ret_val != 0)  
                                {   
                                    fprintf(stderr, "semop R4 deincrement failed ....\a\n");   
                                }
 
                               
                                printf("    R4 starts reading the message:");  
                                printf("%s\n", p_shm->message); 
                                sleep_time = READER_TIME_02;
                                msleep(sleep_time);   
                                printf("    R4 finishes reading the posted message...\n");

                               
                                operations[0].sem_num = 0;  
                                operations[0].sem_op  = 1;  
                                operations[0].sem_flg = 0;  
                                ret_val = semop(sem_id_2, operations, 1);   
                                if (ret_val != 0)
                                {
                                    fprintf(stderr, "semop R4 increment failed ....\a\n");    
                                }
                                //Does opposite of the one above
                                operations[0].sem_num = 0;
                                operations[0].sem_op = -1;   
                                operations[0].sem_flg = 0;  
                                ret_val = semop(sem_id_6, operations, 1);         
                                if (ret_val != 0)  
                                {   
                                    fprintf(stderr, "R4 Sem6 Failed ....\a\n");   
                                } 
                                //***Critical Section Ends***
                        }
                        //Deincriment Children 
                        operations[0].sem_num = 0;
                        operations[0].sem_op = -1;   
                        operations[0].sem_flg = 0;  
                        ret_val = semop(sem_id_5, operations, 1);         
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "R4 Deincrement Child Termination Failed ....\a\n");   
                        } 
				}
				else{
					//Reader 3
                    int sleep_time;
                    if(p_shm->processCount == 5){
                        //Tells everyone to start
                        operations[0].sem_num = 0;
                        operations[0].sem_op = -1;  
                        operations[0].sem_flg = 0; 
                        ret_val = semop(sem_id_3, operations, 1);
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "Start mutex(Readers) has failed....\a\n");   
                        }
                    }
                    else{
                        p_shm->processCount++;
                        operations[0].sem_num = 0;
                        operations[0].sem_op = 0;  
                        operations[0].sem_flg = 0; 
                        ret_val = semop(sem_id_3, operations, 1);
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "Wait until start mutex has failed....\a\n");   
                        }
                    }
                    while(p_shm->Done_Flag[0]!= 1 || p_shm->Done_Flag[1]!=1)   
                        {
                            sleep_time = rand() % READER_TIME_01;
                            msleep(sleep_time);
                            printf("    R3 would like to read the posted message...\n");
                            //***Critical Section Begins***
                            //MUTEX: Only lets reader enter if nobody is writing
                            mutexOperations[0].sem_num = 0;
                            mutexOperations[0].sem_op = 0;   
                            mutexOperations[0].sem_flg = 0;  
                            ret_val = semop(sem_id_1, mutexOperations, 1);         
                            if (ret_val != 0)  
                            {   
                                fprintf(stderr, "Initial Mutex R3 Failed ....\a\n");   
                            }
                            //Does opposite of the one below
                            operations[0].sem_num = 0;
                            operations[0].sem_op = 1;   
                            operations[0].sem_flg = 0;  
                            ret_val = semop(sem_id_6, operations, 1);         
                            if (ret_val != 0)  
                            {   
                                fprintf(stderr, "R3 Sem6 Increment Failed ....\a\n");   
                            }    
                            //AVAILABLE READERS COUNT
                            operations[0].sem_num = 0;
                            operations[0].sem_op = -1;   
                            operations[0].sem_flg = 0;  
                            ret_val = semop(sem_id_2, operations, 1);         
                            if (ret_val != 0)  
                            {   
                                fprintf(stderr, "semop R3 deincrement failed ....\a\n");   
                            }
    
                            
                            printf("    R3 starts reading the message:");  
                            printf("%s\n", p_shm->message); 
                            sleep_time = READER_TIME_02;
                            msleep(sleep_time);   
                            printf("    R3 finishes reading the posted message...\n");

                            
                            operations[0].sem_num = 0;  
                            operations[0].sem_op  = 1;  
                            operations[0].sem_flg = 0;  
                            ret_val = semop(sem_id_2, operations, 1);   
                            if (ret_val != 0)
                            {
                                fprintf(stderr, "semop R3 increment failed ....\a\n");    
                            }
                            //Does opposite of the one above
                            operations[0].sem_num = 0;
                            operations[0].sem_op = -1;   
                            operations[0].sem_flg = 0;  
                            ret_val = semop(sem_id_6, operations, 1);         
                            if (ret_val != 0)  
                            {   
                                fprintf(stderr, "R3 Sem6 Failed ....\a\n");   
                            } 
                            //***Critical Section Ends***
                    }
                    //Deincriment Children 
                    operations[0].sem_num = 0;
                    operations[0].sem_op = -1;   
                    operations[0].sem_flg = 0;  
                    ret_val = semop(sem_id_5, operations, 1);         
                    if (ret_val != 0)  
                    {   
                        fprintf(stderr, "R3 Deincrement Child Termination Failed ....\a\n");   
                    } 
				}
			}
			else{
				//Reader 2
                int sleep_time;
                if(p_shm->processCount == 5){
                    //Tells everyone to start
                    operations[0].sem_num = 0;
                    operations[0].sem_op = -1;  
                    operations[0].sem_flg = 0; 
                    ret_val = semop(sem_id_3, operations, 1);
                    if (ret_val != 0)  
                    {   
                        fprintf(stderr, "Start mutex(Readers) has failed....\a\n");   
                    }
                }
                else{
                    p_shm->processCount++;
                    operations[0].sem_num = 0;
                    operations[0].sem_op = 0;  
                    operations[0].sem_flg = 0; 
                    ret_val = semop(sem_id_3, operations, 1);
                    if (ret_val != 0)  
                    {   
                        fprintf(stderr, "Wait until start mutex has failed....\a\n");   
                    }
                }
                while(p_shm->Done_Flag[0]!= 1 || p_shm->Done_Flag[1]!=1)   
                    {
                    sleep_time = rand() % READER_TIME_01;
                    msleep(sleep_time);
                    printf("    R2 would like to read the posted message...\n");
                        //***Critical Section Begins***
                        //MUTEX: Only lets reader enter if nobody is writing
                        mutexOperations[0].sem_num = 0;
                        mutexOperations[0].sem_op = 0;   
                        mutexOperations[0].sem_flg = 0;  
                        ret_val = semop(sem_id_1, mutexOperations, 1);         
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "Initial Mutex R2 Failed ....\a\n");   
                        }
                        //Does opposite of the one below
                        operations[0].sem_num = 0;
                        operations[0].sem_op = 1;   
                        operations[0].sem_flg = 0;  
                        ret_val = semop(sem_id_6, operations, 1);         
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "R2 Sem6 Increment Failed ....\a\n");   
                        } 
                        //AVAILABLE READERS COUNT
                        operations[0].sem_num = 0;
                        operations[0].sem_op = -1;   
                        operations[0].sem_flg = 0;  
                        ret_val = semop(sem_id_2, operations, 1);         
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "semop R2 deincrement failed ....\a\n");   
                        }
    
                        
                        printf("    R2 starts reading the message:");  
                        printf("%s\n", p_shm->message); 
                        sleep_time = READER_TIME_02;
                        msleep(sleep_time);   
                        printf("    R2 finishes reading the posted message...\n");

                        operations[0].sem_num = 0;  
                        operations[0].sem_op  = 1;  
                        operations[0].sem_flg = 0;  
                        ret_val = semop(sem_id_2, operations, 1);   
                        if (ret_val != 0)
                        {
                            fprintf(stderr, "semop R2 increment failed ....\a\n");    
                        }
                        //Does opposite of the one above
                        operations[0].sem_num = 0;
                        operations[0].sem_op = -1;   
                        operations[0].sem_flg = 0;  
                        ret_val = semop(sem_id_6, operations, 1);         
                        if (ret_val != 0)  
                        {   
                            fprintf(stderr, "R2 Sem6 Failed ....\a\n");   
                        } 
                        //***Critical Section Ends***
                }
                //Deincriment Children 
                operations[0].sem_num = 0;
                operations[0].sem_op = -1;   
                operations[0].sem_flg = 0;  
                ret_val = semop(sem_id_5, operations, 1);         
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "R2 Deincrement Child Termination Failed ....\a\n");   
                } 
			}
		}
		else{
            //Writer 2
			int sleep_time;
            char my_message[256];
            strcpy(my_message,"Hello, I am W2...");
            if(p_shm->processCount == 5){
                //Tells everyone to start
                operations[0].sem_num = 0;
                operations[0].sem_op = -1;  
                operations[0].sem_flg = 0; 
                ret_val = semop(sem_id_3, operations, 1);
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "Start mutex(Readers) has failed....\a\n");   
                }
            }
            else{
                p_shm->processCount++;
                operations[0].sem_num = 0;
                operations[0].sem_op = 0;  
                operations[0].sem_flg = 0; 
                ret_val = semop(sem_id_3, operations, 1);
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "Wait until start mutex has failed....\a\n");   
                }
            }

            for (i = 0; i < NUM_REPEAT; i++)   
                {
                sleep_time = rand() % WRITER_TIME_01;
                msleep(sleep_time);
                printf("W2 would like to post the message...\n");
                mutexOperations[0].sem_num = 0;
                mutexOperations[0].sem_op =  0;  
                mutexOperations[0].sem_flg = 0;        
                //***Critical Section Begins***
                //MUTEX: Writer should (keyword should) not allow any readers or the other writer to go while its writing. 
                mutexOperations[1].sem_num = 0;
                mutexOperations[1].sem_op =  1;  
                mutexOperations[1].sem_flg = 0;  
                ret_val = semop(sem_id_1, mutexOperations, 2);         
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "W1 Mutex Deincrement failed ....\a\n");   
                }
                //Waits for readers to be kicked out
                operations[0].sem_num = 0;
                operations[0].sem_op = 0;   
                operations[0].sem_flg = 0;  
                ret_val = semop(sem_id_6, operations, 1);         
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "Wait for reader to get kicked Failed ....\a\n");   
                } 
                printf("W2 starts posting a new message:");
                printf(" %s\n", my_message); 
                strcpy(p_shm->message,my_message);
                sleep_time = WRITER_TIME_02;
                msleep(sleep_time);   
                printf("    W2 finishes posting a new message...\n");
                // SIGNAL on the semaphore ----
                mutexOperations[0].sem_num = 0;  
                mutexOperations[0].sem_op  = -1;  
                mutexOperations[0].sem_flg = 0;  
                ret_val = semop(sem_id_1,  mutexOperations, 1);   
                if (ret_val != 0)
                {
                    fprintf(stderr, "W2 Mutex Increment failed....\a\n");    
                }  
            }
            p_shm->Done_Flag[1]=1;
            //Deincriment Children 
            operations[0].sem_num = 0;
            operations[0].sem_op = -1;   
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id_5, operations, 1);         
            if (ret_val != 0)  
            {   
                fprintf(stderr, "W2 Deincrement Child Termination Failed ....\a\n");   
            }            
		}
	}
	else{
		//Writer 1
        int sleep_time;
        char my_message[256];
        strcpy(my_message,"Hello, I am W1...");
        if(p_shm->processCount == 5){
            operations[0].sem_num = 0;
            operations[0].sem_op = -1;  
            operations[0].sem_flg = 0; 
            ret_val = semop(sem_id_3, operations, 1);
            if (ret_val != 0)  
            {   
                fprintf(stderr, "Start mutex(Readers) has failed....\a\n");   
            }
        }
        else{
            p_shm->processCount++;
            operations[0].sem_num = 0;
            operations[0].sem_op = 0;  
            operations[0].sem_flg = 0; 
            ret_val = semop(sem_id_3, operations, 1);
            if (ret_val != 0)  
            {   
                fprintf(stderr, "Wait until start mutex has failed....\a\n");   
            }
        }

        for (i = 0; i < NUM_REPEAT; i++)   
            {
                sleep_time = rand() % WRITER_TIME_01;
                msleep(sleep_time);
                printf("W1 would like to post the message...\n");
                mutexOperations[0].sem_num = 0;
                mutexOperations[0].sem_op =  0;  
                mutexOperations[0].sem_flg = 0;        
                //***Critical Section Begins***
                //MUTEX: Writer should (keyword should) not allow any readers or the other writer to go while its writing. 
                mutexOperations[1].sem_num = 0;
                mutexOperations[1].sem_op =  1;  
                mutexOperations[1].sem_flg = 0;  
                ret_val = semop(sem_id_1, mutexOperations, 2);         
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "W1 Mutex Deincrement failed ....\a\n");   
                }
                //Waits for readers to be kicked out
                operations[0].sem_num = 0;
                operations[0].sem_op = 0;   
                operations[0].sem_flg = 0;  
                ret_val = semop(sem_id_6, operations, 1);         
                if (ret_val != 0)  
                {   
                    fprintf(stderr, "Wait for reader to get kicked Failed ....\a\n");   
                } 
                printf("W1 starts posting a new message:");
                printf(" %s\n", my_message); 
                strcpy(p_shm->message,my_message);
                sleep_time = WRITER_TIME_02;
                msleep(sleep_time);   
                printf("    W1 finishes posting a new message...\n");
                // SIGNAL on the semaphore ----
                mutexOperations[0].sem_num = 0;  
                mutexOperations[0].sem_op  = -1;  
                mutexOperations[0].sem_flg = 0;  
                ret_val = semop(sem_id_1, mutexOperations, 1);   
                if (ret_val != 0)
                {
                    fprintf(stderr, "W1 Mutex Increment failed....\a\n");    
                }  
            }
            p_shm->Done_Flag[0]=1;
            //Deincriment Children 
            operations[0].sem_num = 0;
            operations[0].sem_op = -1;   
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id_5, operations, 1);         
            if (ret_val != 0)  
            {   
                fprintf(stderr, "W1 Deincrement Child Termination Failed ....\a\n");   
            }
	    }
  }
   else{
    //Parent/R1
    int sleep_time;
    if(p_shm->processCount == 5){
        //Tells everyone to start
        operations[0].sem_num = 0;
        operations[0].sem_op = -1;  
        operations[0].sem_flg = 0; 
        ret_val = semop(sem_id_3, operations, 1);
        if (ret_val != 0)  
        {   
            fprintf(stderr, "Start mutex(Readers) has failed....\a\n");   
        }
    }
    else{
        p_shm->processCount++;
        operations[0].sem_num = 0;
        operations[0].sem_op = 0;  
        operations[0].sem_flg = 0; 
        ret_val = semop(sem_id_3, operations, 1);
        if (ret_val != 0)  
        {   
            fprintf(stderr, "Wait until start mutex has failed....\a\n");   
        }
    }
	while(p_shm->Done_Flag[0]!= 1 || p_shm->Done_Flag[1]!=1)   
        {
	       sleep_time = rand() % READER_TIME_01;
	       msleep(sleep_time);
	       printf("    R1 would like to read the posted message...\n");
            //***Critical Section Begins***
            //MUTEX: Only lets reader enter if nobody is writing
            mutexOperations[0].sem_num = 0;
            mutexOperations[0].sem_op = 0;   
            mutexOperations[0].sem_flg = 0;  
            ret_val = semop(sem_id_1, mutexOperations, 1);         
            if (ret_val != 0)  
            {   
                fprintf(stderr, "Initial Mutex R1 Failed ....\a\n");   
            }
            //Does opposite of the one below
            operations[0].sem_num = 0;
            operations[0].sem_op = 1;   
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id_6, operations, 1);         
            if (ret_val != 0)  
            {   
                fprintf(stderr, "R1 Sem6 Increment Failed ....\a\n");   
            } 
            //AVAILABLE READERS COUNT
	        operations[0].sem_num = 0;
            operations[0].sem_op = -1;   
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id_2, operations, 1);        
            if (ret_val != 0)  
            {   
                fprintf(stderr, "semop R1 deincrement failed ....\a\n");   
            }

            printf("    R1 starts reading the message:");  
            printf("%s\n", p_shm->message); 
	        sleep_time = READER_TIME_02;
	        msleep(sleep_time);   
	        printf("    R1 finishes reading the posted message...\n");
 
            operations[0].sem_num = 0;  
            operations[0].sem_op  = 1;  
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id_2, operations, 1);   
            if (ret_val != 0)
            {
                fprintf(stderr, "semop R1 increment failed ....\a\n");    
            }
            //Does opposite of the one above
            operations[0].sem_num = 0;
            operations[0].sem_op = -1;   
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id_6, operations, 1);         
            if (ret_val != 0)  
            {   
                fprintf(stderr, "R1 Sem6 Failed ....\a\n");   
            }
            //***Critical Section Ends***
	}
        operations[0].sem_num = 0;
        operations[0].sem_op = 0;   
        operations[0].sem_flg = 0;  
        ret_val = semop(sem_id_5, operations, 1);         
        if (ret_val != 0)  
        {   
            fprintf(stderr, "Wait for Child Termination Failed ....\a\n");   
        }

        // detach the shared memory ---
        ret_val = shmdt(p_shm);  
        if (ret_val != 0) 
        {  printf ("shared memory detach failed ....\n"); }

        ret_val = shmctl(shm_id, IPC_RMID, 0); 
        if (ret_val != 0)
        {  printf("shared memory ID remove ID failed ... \n"); } 

        ret_val = semctl(sem_id_1, IPC_RMID, 0);  
        if (ret_val != 0)
        {  printf("semaphore remove ID failed ... \n"); }
 
        ret_val = semctl(sem_id_2, IPC_RMID, 0);  
        if (ret_val != 0)
        {  printf("semaphore remove ID failed ... \n"); }
 
        ret_val = semctl(sem_id_3, IPC_RMID, 0);  
        if (ret_val != 0)
        {  printf("semaphore remove ID failed ... \n"); }
 
        // ret_val = semctl(sem_id_4, IPC_RMID, 0);  
        // if (ret_val != 0)
        // {  printf("semaphore remove ID failed ... \n"); }
        ret_val = semctl(sem_id_5, IPC_RMID, 0);  
        if (ret_val != 0)
        {  printf("semaphore remove ID failed ... \n"); }
        ret_val = semctl(sem_id_6, IPC_RMID, 0);  
        if (ret_val != 0)
        {  printf("semaphore remove ID failed ... \n"); }

        exit(0);         
   } 
}


void msleep(unsigned micro_seconds)
{ 
     unsigned int RAND_FACTOR;
     unsigned int RAND_SLEEP_TIME;

     RAND_FACTOR = uniform_rand();
     RAND_SLEEP_TIME = (int)((float)RAND_FACTOR / ((float)(100.0)) * (float)(micro_seconds));

     usleep(RAND_SLEEP_TIME);
}

// uniform_rand ///////////////////////////////////////////////////////////////
unsigned int uniform_rand(void)
/* generate a random number 0 ~ 99 */
{
    unsigned int my_rand;
    my_rand = rand() % 100;

    return (my_rand);
}