// Tala_1201107 ^ Lana_1200308
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <termios.h>
#include <fcntl.h>

#define BUFFER_SIZE 10
#define MAX_LENGTH 10
#define SMKEY 1234
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

typedef struct {
    char data[BUFFER_SIZE][MAX_LENGTH];
    int in;
    int out;
    int count;
    int locked;
} CircularBuffer;
// void consumer(CircularBuffer *buffer){
//     while(1){
//          if(!paused)
//         { 
//         if (!buffer->locked) { // check if the buffer is unlocked
//             buffer->locked = 1; // lock the buffer
//              if (buffer->count == 0) {
//                         // if (count >= BUFFER_SIZE) 
//                 printf("Circular buffer is empty. Waiting for producer to add items...\n");
//                 buffer->locked = 0; // unlock the buffer
//                 sleep(1); // wait for producer to add items        
//                     } 
       
     
//     }



//         }
//     }
// }
int main() {
    int shmid;
    CircularBuffer *buffer;
    char ch;

    shmid = shmget(SMKEY, sizeof(BUFFER_SIZE), 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    buffer = (CircularBuffer *) shmat(shmid, NULL, 0);
    if (buffer == (CircularBuffer *) -1) {
        perror("shmat");
        exit(1);
    }


    while (1) {
        if(kbhit())
        {
            char ch = getchar();
            if(ch == 'c')
            {
                paused = !paused; // toggle pause
               printf("PAUSE/RESUME\n");
            }
        }
         if(!paused)
        { 
        if (!buffer->locked) { // check if the buffer is unlocked
            buffer->locked = 1; // lock the buffer
            if (buffer->count <= 0) {
                printf("Circular buffer is empty. Waiting for producer to add items...\n");
                buffer->locked = 0; // unlock the buffer
                sleep(2); // wait for producer to add items
            } else {
                buffer->out = (buffer->out + 1) % BUFFER_SIZE;
                buffer->count--;
                printf("Consumed item from buffer.\n");
              //  buffer->locked = 0; // unlock the buffer
               // break; // exit the loop
            }
                   buffer->locked = 0; // unlock the buffer
            sleep(2); // wait for 2 second before consuming another item
        }
        // }if(kbhit())
        // {
        //     char ch = getchar();
        //     if(ch == 'c')
        //     {
        //         paused = !paused; // toggle pause
        //        if(paused)
        //         printf("consumer paused\n");
        //         else
        //         printf("consumer resumed\n");
        //     }
        // }
    }



    if (shmdt(buffer) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
