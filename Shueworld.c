#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 

//Function headers
void* shoeThread(void* arg);
void randomSleep(void);
void shoeArrives(int shoe_id, int shoe_type);
void shoesLeave(int shoe_id, int shoe_type);
void insertShoe(int type, int id);

//Constants
const int runner = 18;
const int dress = 16;
const int crossover = 15;
const int total_shoes = runner + dress + crossover;
const int run_id = 0;
const int dress_id = 1;
const int crossover_id = 2;
const int amounts[3] = {18, 16, 15};
int full[3] = {0, 0, 0};
int ready[3] = {0, 0, 0};
const int stage_capacity = 6; 

//Global variables
int current_stage_type = -1;  
int shoes_on_stage = 0;      
int stage[2][6] = {{-1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1}};
int counts[3] = {0, 0, 0};

//Mutex and condition variables
pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t type_ready = PTHREAD_COND_INITIALIZER;
pthread_cond_t stage_open;
pthread_cond_t need_friends;

//Struct for shoe data
typedef struct {
    int shoe_id;
    int shoe_type;
} ShoeData;


int main(void){
    // Read seed value from file
    FILE *fp;
    int rseed;
    fp = fopen("seed.txt","r");
    fscanf(fp, "%d", &rseed);
    fclose(fp);
    printf("Read seed value: %d\n", rseed);
    srand(rseed);
    
    // Initialize threads
    pthread_t threads[total_shoes];
    ShoeData shoe_data[total_shoes];

    // Initialize mutex and condition variables
    pthread_mutex_init(&stage_lock, NULL);
    pthread_cond_init(&stage_open, NULL);

    int thread_index = 0;

    // Create threads for runner shoe
    for (int i = 0; i < runner; i++, thread_index++) {
        shoe_data[thread_index].shoe_id = i + 1;
        shoe_data[thread_index].shoe_type = run_id;
        pthread_create(&threads[thread_index], NULL, shoeThread, &shoe_data[thread_index]);
    }

    // Create threads for dress shoe
    for (int i = 0; i < dress; i++, thread_index++) {
        shoe_data[thread_index].shoe_id = i + 1;
        shoe_data[thread_index].shoe_type = dress_id;
        pthread_create(&threads[thread_index], NULL, shoeThread, &shoe_data[thread_index]);
    }

    // Create threads for crossover shoe
    for (int i = 0; i < crossover; i++, thread_index++) {
        shoe_data[thread_index].shoe_id = i + 1;
        shoe_data[thread_index].shoe_type = crossover_id;
        pthread_create(&threads[thread_index], NULL, shoeThread, &shoe_data[thread_index]);
    }

    // Join threads
    for (int i = 0; i < total_shoes; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}

//Function to sleep for a random amount of time
void randomSleep(void){
    int sleeptime = (rand() % 20)+1;
    sleep(sleeptime);
}

//Function to sleep for a random amount of time
void randomStageSleep(void){
    int sleeptime = (rand() % 10)+1;
    sleep(sleeptime);
}

//Function to handle shoe arrival
void* shoeThread(void* arg){
    //Get shoe data
    ShoeData *data = (ShoeData*) arg;
    int shoe_id = data->shoe_id;
    int shoe_type = data->shoe_type;

    //Shoe arrives
    while(1){
        randomSleep();

        if(counts[shoe_type] >= amounts[shoe_type]){
            full[shoe_type] = 1;
        }

        //All Shoes have equal access to the stage
        if(((full[0] == 1) && (full[1] == 1)) && (full[2] == 1)){
            printf("FREE FOR ALLLLLL!!!\n");
            full[0] = 0;
            full[1] = 0;
            full[2] = 0;
            counts[0] = 0;
            counts[1] = 0;
            counts[2] = 0;
        }

        //Lock the stage
        pthread_mutex_lock(&stage_lock); 

        //Attempt to join the stage
        while(shoes_on_stage >= stage_capacity 
        || ((current_stage_type != -1 && current_stage_type != shoe_type) || (full[shoe_type] == 1))){
            pthread_cond_wait(&stage_open, &stage_lock);
        }

        if (shoe_type != current_stage_type) {
            ready[shoe_type]++;
            while(ready[shoe_type] < 2){
                pthread_cond_wait(&need_friends, &stage_lock);
            }

            pthread_cond_signal(&need_friends);
            ready[shoe_type]--;
        }

        //Print if shoe could get on stage
        if(current_stage_type == -1){
            insertShoe(shoe_type, shoe_id);
            current_stage_type = shoe_type;
            if (shoe_type == run_id) {
                printf("Running Shoe #%d is on stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
            } else if (shoe_type == dress_id) {
                printf("Dress Shoe #%d is on stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
            } else {
                printf("Crossover Shoe #%d is on stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
            }
        }
        else{
            insertShoe(shoe_type, shoe_id);
            if (shoe_type == run_id) {
                printf("Running Shoe #%d is on stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
            } else if (shoe_type == dress_id) {
                printf("Dress Shoe #%d is on stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
            } else {
                printf("Crossover Shoe #%d is on stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
            }
        }

        //Print current stage array
        printf("Current stage array:\n");
            for (int i = 0; i < stage_capacity; i++) {
                printf("[%d, %d] ", stage[0][i], stage[1][i]);
            }
        printf("\n");

        pthread_mutex_unlock(&stage_lock);
        randomStageSleep();
        shoesLeave(shoe_id, shoe_type);
    }
    return NULL;
}

//Function to handle shoe leaving
void shoesLeave(int shoe_id, int shoe_type){
    pthread_mutex_lock(&stage_lock);

    //Remove shoe from stage
    for(int i = 0; i < stage_capacity; i++){
        if(stage[1][i] == shoe_id){
            shoes_on_stage--;
            if(shoe_type == run_id){
                printf("Running Shoe #%d left the stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
                counts[0]++;
            } else if(shoe_type == dress_id){
                printf("Dress Shoe #%d left the stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
                counts[1]++;
            } else {
                printf("Crossover Shoe #%d left the stage (%d shoes on stage)\n", shoe_id, shoes_on_stage);
                counts[2]++;
            }
            stage[0][i] = -1;
            stage[1][i] = -1;
            printf("Current stage array:\n");
            for (int i = 0; i < stage_capacity; i++) {
                printf("[%d, %d] ", stage[0][i], stage[1][i]);
            }
            printf("\n");
            break;
        }
    }

    //Signal that stage is open
    for (int i = 0; i < stage_capacity; i++) {
        if (stage[0][i] != -1) {
            break;
        }
        if (i == stage_capacity - 1) {
            printf("STAGE OPENNNNN\n");
            current_stage_type = -1;
            pthread_cond_broadcast(&stage_open);
        }
    }
    pthread_mutex_unlock(&stage_lock);
}

//Function to handle shoe arrival on stage
void insertShoe(int type, int id){
    for(int i = 0; i < stage_capacity; i++){
        if(stage[1][i] == -1){
            stage[0][i] = type;
            stage[1][i] = id;
            break;
        }
    }
    shoes_on_stage++;
}