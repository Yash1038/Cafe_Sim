## Concurrency

### 1. Cafe Simulator

- Assumptions
    - The cafe has following limitations such as 100 Baristas, Customers, and Coffee types
    - The maximum length of a coffee type does not exceed 20 characters
- Wait Time is calculated as follows:
    - If the customer leaves before his order is even allocated, then wait time is his tolerance/patience
    - If the customer leaves after his order is allocated (irrespective of delivered or not), then wait time is the time taken until the order was accepted/started preparing
- The cafe is implemented using the following data structures:
    - `struct customer` - contains the customer's name, order, and tolerance
    - `struct coffee_type`  - contains the coffee type
- `struct customer` has the following data in it as shown below
```c
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
```

- At the end of the simulation, if the customer status shows 1, then he has left the cafe without his order being delivered, but has been allocated

- Therefore, the total number of coffees wasted is the number of customers with status 1

- Total wait time of the customers is the sum of the wait time of all the customers

- If there were infinite number of baristas, then the total wait time would be 0, as the customers would be served as soon as they enter the cafe.

- This simulator is built with the help of pthreads and semaphores.