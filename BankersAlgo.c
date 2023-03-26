
#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

int total[NUMBER_OF_RESOURCES]={0};
int available[NUMBER_OF_RESOURCES];
int available2[NUMBER_OF_RESOURCES];
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int maximum2[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES]={0};
int allocation2[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need2[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int Done[NUMBER_OF_CUSTOMERS]={0};

int release_resources(int customer_num);
int request_resources(int customer_num, int request[]);
void *Threads(void* customer_numt);
int bankerAlgorithm(int customer_num,int request[]);
void OutputState();
pthread_mutex_t mutex;
char string[NUMBER_OF_RESOURCES*2]={0};
int isSafe[NUMBER_OF_CUSTOMERS]={0};


int main(int argc, const char * argv[]) {

    //check to see if the user inputs the right number of arguments 

    if (argc != NUMBER_OF_RESOURCES + 1){
        printf("Incorrect Number of Arguments! Please pass 3 integer arguments.\n");
        return EXIT_FAILURE;
    }


int arg1 = atoi(argv[1]);
int arg2 = atoi(argv[2]);
int arg3 = atoi(argv[3]);

    //check to see if the user inputs arguments greater than 0 

    if (arg1 <= 0 || arg2 <= 0 || arg3 <= 0) {
        printf("Please pass in arguments that are greater than 0.\n");
        return 1;
    }
    
  //initialize the matrices 
    for(int i=0 ;i<argc-1;i++){
        available[i]=atoi(argv[i+1]);
        // At the start available resources equal to total resources
        total[i]=available[i];      
    }
    for(int i=0;i< NUMBER_OF_RESOURCES;i++){
        for(int j=0;j<NUMBER_OF_CUSTOMERS;j++){
            //maximum should less than total
            maximum[j][i]=rand()%(total[i]+1);     
            need[j][i]=maximum[j][i];    // need=maximum-allocation  (allocation=0)
        
        }
    
    }
    
    //print the state of the process 
    
    printf("The total system resources are:\n");
    printf("%s\n",string);
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        printf("%d ",total[i]);
    }
    printf("\n\nMaximum resources):\n");
    printf("   %s\n",string);
    for(int i=0; i<NUMBER_OF_CUSTOMERS;i++){
        printf("P%d ",i+1);
        for(int j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",maximum[i][j]);
        }
        printf("\n");
    }

   //print the state of the process

    OutputState();
    
    
    //initialize the mutex
   pthread_mutex_init(&mutex, NULL);    
   pthread_t p1,p2,p3,p4,p5;
   int a1=0,a2=1,a3=2,a4=3,a5=4;
   //create 5 threads
   pthread_create(&p1,NULL,Threads,&a1);
   pthread_create(&p2,NULL,Threads,&a2);
   pthread_create(&p3,NULL,Threads,&a3);
   pthread_create(&p4,NULL,Threads,&a4);
   pthread_create(&p5,NULL,Threads,&a5);     
   char *Value_Return;
    
    // wait for all the 5 threads to terminate
   pthread_join(p1,(void**)&Value_Return);
   pthread_join(p2,(void**)&Value_Return);
   pthread_join(p3,(void**)&Value_Return);
   pthread_join(p4,(void**)&Value_Return);
   pthread_join(p5,(void**)&Value_Return);
    
    return 0;
}



void *Threads(void* Num_Customers){

    int Sum_Request=0;
    int *c=(int*)Num_Customers;
    int customer_num= *c;
    
    // the while loop stops when the thread has finished and its need becomes zero
    while(!Done[customer_num]){   
        Sum_Request=0;
        int request[NUMBER_OF_RESOURCES]={0};
        
        for(int i=0;i<NUMBER_OF_RESOURCES;i++){
           
           // generate a request below its need randomly
            request[i]=rand()%(need[customer_num][i]+1); 
            Sum_Request += request[i];
        }

        //condition that ensures it does not request for 0 resources 
        if(Sum_Request!=0) 
        // only when the request has been granted succesfully
            while(request_resources(customer_num,request)==-1); 
                                                   
    }
    
    return 0;
}



int request_resources(int customer_num, int request[]) {
    printf("\nP%d requests for ", customer_num + 1);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        printf("%d ", request[i]);
    }
    printf("\n");

    pthread_mutex_lock(&mutex); // acquire the mutex

    // Check if the request can be granted
    int canGrant = 1;
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > available[i]) {
            canGrant = 0;
            break;
        }
    }

    // If the request cannot be granted, release the mutex and return -1
    if (!canGrant) {
        printf("P%d is waiting for the resources...\n", customer_num + 1);
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    // If the request can be granted, try to execute the Banker's algorithm
    int returnValue = bankerAlgorithm(customer_num, request);

    if (returnValue == 0) { // A safe sequence was found
        printf("a safe sequence is found: ");
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            printf("P%d ", isSafe[i] + 1);
        }
        printf("\nP%d's request has been granted\n", customer_num + 1);

        // Update the allocation, available, and need matrices
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            allocation[customer_num][j] += request[j];
            available[j] -= request[j];
            need[customer_num][j] -= request[j];
        }

        // If the thread has finished its tasks, mark it as finished
        int needIsZero = 1;
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            if (need[customer_num][j] != 0) {
                needIsZero = 0;
                break;
            }
        }

        if (needIsZero) {
            // if need is zero, mark the thread "finish"
            Done[customer_num] = 1;
            // release resources when a thread finishes
            release_resources(customer_num);
        }

        OutputState();
    } else {
        printf("cannot find a safe sequence\n");
    }

    pthread_mutex_unlock(&mutex); // release the mutex
    return returnValue;
}



int release_resources(int Num_Customer){
    
    
    printf("P%d releases all the resources\n",Num_Customer+1);
    for(int j=0;j<NUMBER_OF_RESOURCES;j++){
        available[j]=available[j]+allocation[Num_Customer][j]; // release the resources
        allocation[Num_Customer][j]=0;
        
        
    }
    
    
    
    return 0;
}
int bankerAlgorithm(int customer_num,int request[]){
    int finish[NUMBER_OF_CUSTOMERS]={0};
    
    // copy the matrices
    for(int i=0;i<NUMBER_OF_RESOURCES;i++){ 
        available2[i]=available[i];
        for(int j=0;j<NUMBER_OF_CUSTOMERS;j++){
            allocation2[j][i]=allocation[j][i];
            
            maximum2[j][i]=maximum[j][i];
         
            need2[j][i]=need[j][i];
        
        
        }
    }
        // pretend to give the resource to the thread
        for(int i=0;i<NUMBER_OF_RESOURCES;i++){ 
            available2[i]=available2[i]-request[i];
            allocation2[customer_num][i]=allocation2[customer_num][i]+request[i];
            need2[customer_num][i]=need2[customer_num][i]-request[i];
        }
    
   

    
//safety Algorithm
    int count=0;
    do {

               int I=-1;
        
        for(int i=0;i<NUMBER_OF_CUSTOMERS;i++){ // to find a thread that its need is less than or equal to available.
            int condition=1;
            for(int j=0; j<NUMBER_OF_RESOURCES;j++){
                if(need2[i][j]>available2[j] || finish[i]==1){
                    condition=0;
                    break;
                }
                
            }
            if(condition){ // if the thread is found, record its thread number
                I=i;
                break;
            }
            
        }
        
        if(I!=-1){
            isSafe[count]=I; // record the sequence
            count++;
            finish[I]=1; // mark the thread "finish"
            for(int k=0;k<NUMBER_OF_RESOURCES;k++){  // pretend to give the reaource to thread
                available2[k]=available2[k]+allocation2[I][k];
            }
            
            
        }
        else{ // if can not find any thread that its need is less than or equal to available.
            
            for(int i=0;i<NUMBER_OF_CUSTOMERS;i++){
                if(finish[i]==0){ // if there is any thread hasn't been found, that means it can't find a safe sequence
                    return -1;
                }
                
            }
            return 0;  // all the threads have been found at this point 
            
            
        }


    }
    while(1);
        
    
}


//function to print the output of all information required to the user 
void OutputState(){
    
    printf("Currently allocated resources:\n");
    printf("   %s\n",string);
    
    for(int i=0; i<NUMBER_OF_CUSTOMERS;i++){
        printf("P%d ",i+1);
        for(int j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",allocation[i][j]);
        }
        printf("\n");
    }
    printf("\n");


    printf("Maximum resources:\n");
    printf("   %s\n",string);
    
    for(int i=0; i<NUMBER_OF_CUSTOMERS;i++){
        printf("P%d ",i+1);
        for(int j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",maximum[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("Possibly needed resources:\n");
    printf("   %s\n",string);
    for(int i=0; i<NUMBER_OF_CUSTOMERS;i++){
        printf("P%d ",i+1);
        for(int j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",need[i][j]);
        }
        
        printf("\n");
    }
    printf("\n");

    printf("Available system resources are:\n");
    printf("%s\n",string);
    
    for (int i=0; i<NUMBER_OF_RESOURCES; i++) {
        printf("%d ",available[i]);
    }
    
    printf("\n");
    
    
    
}


