// Tala_1201107 & Lana_1200308
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>  // for fcntl()
#include <errno.h>  // for errno
#include <termios.h>
#include <fcntl.h>
#define BUFFER_SIZE 10 // size of shared memory 
#define MAX_LENGTH 10 // num of characters 
#define SMKEY 1234 // shared memory key 
#define MAX_ITEMS 10 // maximum number of items to add to shared memory 

typedef struct {
    char data[BUFFER_SIZE][MAX_LENGTH];
    int in;    // index where the producer will insert the next item
    int out;   // index where the consumer will remove the next item
    int count; // number of items that have been added to the buffer
    int locked; // lock the memory while being used 
} CircularBuffer;
    int paused = 0; // initially not paused


int kbhit(void)
{
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void producer(CircularBuffer *buffer) {
//int count
int count1 = 0;

    while (1) {

        if(!paused)
        { 
        if (!buffer->locked) { // check if the buffer is unlocked
            buffer->locked = 1; // lock the buffer
             if (buffer->count >= MAX_ITEMS) {
                        // if (count >= BUFFER_SIZE) {
                printf("Circular buffer is full.\n");
                 buffer->locked = 0; // unlock the buffer
                 sleep(2); //wait for consumer
            } else {
                if (buffer->in == -1) { 
                    buffer->in = 0;
                    buffer->out = 0;
                } else {
                    buffer->in = (buffer->in + 1) % BUFFER_SIZE;
                }
                // generate a random string
                char data[MAX_LENGTH];
                for (int i = 0; i < MAX_LENGTH - 1; i++) {
                    data[i] = 'a' + rand() % 26; // generates a random lowercase letter
                }
                data[MAX_LENGTH - 1] = '\0'; // terminate the string
                strncpy(buffer->data[buffer->in], data, MAX_LENGTH);
                count1++;

                buffer->count = count1;
                // count1++;
              //  buffer->count++;
                // = count1;
               printf("count %d\n",buffer->count);
                printf("Added item to buffer: %s\n", data);
            }
            buffer->locked = 0; // unlock the buffer
            sleep(1); // wait for 1 second before adding another item
        }
        }
        if(kbhit())
        {
            char ch = getchar();
            if(ch == 'p')
            {
                paused = !paused; // toggle pause
               printf("PAUSE/RESUME\n");
            }
        }
    }
}

 
int main() {
    
    int shmid;
    CircularBuffer *buffer;

    shmid = shmget(SMKEY, sizeof(BUFFER_SIZE), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    buffer = (CircularBuffer *) shmat(shmid, NULL, 0);
    if (buffer == (CircularBuffer *) -1) {
        perror("shmat");
        exit(1);
    }




    buffer->in = -1;
    buffer->out = -1;
   // buffer->count = 0;
    buffer->locked = 0;///?


    // seed the random number generator
    srand(time(NULL));

    pid_t pid  = fork();
    if (pid < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // We are in the child process
        execl("./consumeer", "./consumeer", NULL);
        // If execl returns, it means the call has failed
        perror("execl() failed");
        exit(EXIT_FAILURE);
    } else {
       
          producer(buffer);
        // We are in the parent process
        wait(NULL);
    }


    if (shmdt(buffer) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
