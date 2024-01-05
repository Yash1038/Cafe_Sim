// Cafe Simulator

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Colors for Output

#define C_WHITE "\033[0m"
#define C_YELLOW "\033[0;33m"
#define C_RED "\033[0;31m"
#define C_GREEN "\033[0;32m"
#define C_BLUE "\033[0;34m"
#define C_CYAN "\033[0;36m"
#define C_RESET "\033[0m"


#define ff(i,a,b) for(int i = a;i < b; i++)


// Limitations

#define MAX_CUSTOMERS 100
#define MAX_BARISTAS 100
#define MAX_COFFEE_TYPES 100

// Structures

typedef struct coffee_type{
    char name[20];
    int time;
} Cofee_Type;

typedef struct customer{
    int id;
    char order[20];
    int entry;
    int max_exit;
    int mak_tim;      // This basically takes the time of execution for making that coffee
    int tim_wait;     // This is basically time he waits in the queue
    int status;        // Stores if coffee making has started or not (2 for given, 1 for started and 0 for not started)
    int left;         // Stores if left or not (1 for left and 0 for not left)
} Customer;

// More Global arrays
Cofee_Type cf_typ[MAX_COFFEE_TYPES];
Customer cust[MAX_CUSTOMERS];
int Barista_assign[MAX_BARISTAS];

// Global integers
int B, K, N;
time_t clock1;
sem_t baristas;        // Semaphore for baristas
pthread_mutex_t barista_assign_lock = PTHREAD_MUTEX_INITIALIZER;

// Bubble sort function to sort the customers according to their entry time, if same entry time, then sort according to their id

void bubble_sort(Customer *cust, int n){
    int i, j;
    Customer temp;
    for(i = 0; i < n-1; i++){
        for(j = 0; j < n-i-1; j++){
            if(cust[j].entry > cust[j+1].entry){
                temp = cust[j];
                cust[j] = cust[j+1];
                cust[j+1] = temp;
            }
            else if(cust[j].entry == cust[j+1].entry){
                if(cust[j].id > cust[j+1].id){
                    temp = cust[j];
                    cust[j] = cust[j+1];
                    cust[j+1] = temp;
                }
            }
        }
    }
}

void *customer_wait(void *arg){
    while(1){
        Customer *cust = (Customer *)arg;

        if(cust->left == 1){
            pthread_exit(&(cust->id));
        }

        if(cust->status == 2){
            pthread_exit(&(cust->id));
        }
        time_t temp = time(NULL);

        if(temp - cust->entry - clock1 > cust->max_exit){
            printf(C_RED "Customer %d leaves without their order at %ld second(s) \n", cust->id, temp - clock1);
            cust->left = 1;
            pthread_exit(&(cust->id));
        }

        sleep(1);
    }
}

void *customer(void *arg){
    Customer *cust = (Customer *)arg;
    int id = cust->id;
    int entry = cust->entry;
    int max_exit = cust->max_exit;
    int mak_tim = cust->mak_tim;
    int tim_wait = cust->tim_wait;
    int status = cust->status;
    int left = cust->left;
    char order[20];
    strcpy(order, cust->order);

    time_t temp = time(NULL);
    printf(C_WHITE "Customer %d arrives at %ld second(s) \n", id, temp - clock1);
    printf(C_YELLOW "Customer %d orders %s\n", id, order);
    pthread_t temp_id;
    pthread_create(&temp_id, NULL, customer_wait, (void *)cust);
    sem_wait(&baristas);
    sleep(1);
    // Sleep for custid * 1 milliseconds
    sleep(id * 0.001);

    // Assigning whichever barista is free
    pthread_mutex_lock(&barista_assign_lock);
    int bar_index = -1;
    ff(i,0,B){
        if(Barista_assign[i] == -1){
            Barista_assign[i] = 1;
            bar_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&barista_assign_lock);

    if(cust->left != 0){
        sem_post(&baristas);
        pthread_exit(&(cust->id));
    }
    cust->status = 1; // Coffee making started
    time_t temp2 = time(NULL);

    // printf(C_CYAN "Customer %d's order of %s is being made by barista %d at time %ld \n", id, order, bar_index + 1,  temp2 - clock1);
    printf(C_CYAN "Barista %d begins preparing the order of Customer %d at %ld second(s)\n", bar_index + 1, id, temp2 - clock1);
    // making changes in the customer structure
    cust->tim_wait += temp2 - entry - clock1;
    sleep(mak_tim);
    // printf(C_BLUE "Customer %d's order of %s is ready at time %ld seconds\n", id, order, time(NULL) - clock1);
    printf(C_BLUE "Barista %d successfully completes the order of customer %d at %ld second(s)\n", bar_index + 1, id, time(NULL) - clock1);
    if(time(NULL) - clock1 - entry <= max_exit){
        cust->status = 2; // Coffee making done
        printf(C_GREEN "Customer %d leaves at %ld with his order\n", id, time(NULL) - clock1);
    }
    // cust->status = 2; // Coffee making done
    
    pthread_mutex_lock(&barista_assign_lock);
    Barista_assign[bar_index] = -1;
    pthread_mutex_unlock(&barista_assign_lock);
    sem_post(&baristas);


}


int main(){

    scanf("%d %d %d", &B, &K, &N);

    // Taking input for the coffee types

    ff(i,0,K){
        scanf("%s %d", cf_typ[i].name, &cf_typ[i].time);
    }

    // Taking input for the customers

    ff(i,0,N){
        scanf("%d %s %d %d", &cust[i].id, cust[i].order, &cust[i].entry, &cust[i].max_exit);

        // Assigning the time for making the coffee
        ff(j,0,K){
            if(strcmp(cust[i].order, cf_typ[j].name) == 0){
                cust[i].mak_tim = cf_typ[j].time;
                break;
            }
        }

        cust[i].tim_wait = 0;
        cust[i].status = 0;
        cust[i].left = 0;
    }

    // Assigning all baristas a value of -1. -1 means that the barista is free, 1 means he is busy
    ff(i,0,B){
        Barista_assign[i] = -1;
    }

    // Printing the input for coffe types in CYAN and for customers in YELLOW

    // ff(i,0,K){
    //     printf(C_CYAN "%s %d\n", cf_typ[i].name, cf_typ[i].time);
    // }

    // ff(i,0,N){
    //     printf(C_YELLOW "%d %s %d %d %d\n", cust[i].id, cust[i].order, cust[i].entry, cust[i].max_exit, cust[i].mak_tim);
    // }

    // Sort the student records

    bubble_sort(cust, N);

    // Initialising the semaphore for baristas
    sem_init(&baristas, 0, B);
    clock1 = time(NULL);

    // Creating threads for customers,
    time_t temp;
    pthread_t tid[N]; // Thread id for customers

    ff(i,0,N){
        temp = time(NULL);
        sleep(cust[i].entry - temp + clock1);
        pthread_create(&tid[i], NULL, customer, (void *)&cust[i]);
    }

    ff(i,0,N){
        pthread_join(tid[i], NULL);
    }

    sem_destroy(&baristas); // Destroying the semaphore

    // Statistics from this period

    // // Printing coffee status 
    // ff(i,0,N){
    //     printf("Customer %d's coffee status is %d\n", cust[i].id, cust[i].status);
    // }

    int coffee_wasted = 0;
    ff(i,0,N){
        if(cust[i].status == 1){
            coffee_wasted++;
        }
    }

    printf(C_RESET);
    printf("\n\n%d coffee wasted\n", coffee_wasted);

    // Wait time is calculated as follows
    // 1. If the customer leaves before his order is even allocated, then wait time is his max exit time
    // 2. If customer leaves after his order is allocated, then wait time is the time he waited until his order was taken

    int total_wait = 0;
    ff(i,0,N){
        if(cust[i].status != 0){
            total_wait += cust[i].tim_wait;
            // printf("Customer %d's wait time is %d\n", cust[i].id, cust[i].tim_wait);
        }else{
            total_wait += cust[i].max_exit;
            // printf("Customer %d's wait time is %d\n", cust[i].id, cust[i].max_exit);
        }
    }

    // printing the average wait time in seconds
    printf("Average wait time is %f seconds\n", (float)total_wait / N);

    return 0;

}