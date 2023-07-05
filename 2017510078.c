#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

//The maximum number of patients threads and healthcare threads.
#define max_NumberofPatients 100
#define max_HealthcareStaff 8

void *Patients(void *num); // This part where patients are transferred to hospital entrance and waiting rooms
void *HealthcareStaff(void *num); //This part includes covid tests of health workers or the process of ventilating the room
void TestTime(int secs); //Time function where frame set for covid test

//Define the semaphores. 

sem_t waitingRoom;//A semaphore that houses the number of patients entering the hospital
sem_t testroom;   //A semaphore created for the number of people needed to avoid experiencing starvation status in test rooms
sem_t StaffStatu; //A semaphore that determines the functioning of the health worker's task
sem_t Preparing;  
sem_t SecondPatient; //A semaphore indicating the second patient entering the room
sem_t ThirdPatient;  //A semaphore indicating the third patient entering the room
sem_t AcceptOnebyOne;//A semaphore that directs patients one by one to 3 capacity rooms

// Flag to stop the Staff thread when all Patients have been serviced.
int allDone = 0;
int count = 0; //The count variable is the variable that holds Max 3 people who must enter the rooms
int closeStatus=0; //closeStatus variable which used to indicate that the duties of health workers within the hospital are over.


int main(int argc, char *argv[])
{
    pthread_t healthcareId[max_HealthcareStaff]; //healthcare thread 
    pthread_t PatientId[max_NumberofPatients];	//patient thread
    int i, numPatientss, numberOfWaitingChair; int Number[max_NumberofPatients]; //define variables

    numPatientss = max_NumberofPatients-1;
    numberOfWaitingChair = 50;  //waiting room capacity

    for (i = 0; i < max_NumberofPatients; i++) {
        Number[i] = i;
    }
    // Initialize the semaphores with initial values...
    sem_init(&waitingRoom, 0, numberOfWaitingChair);
    sem_init(&testroom, 0, max_HealthcareStaff*3);
    sem_init(&StaffStatu, 0, 0);
    sem_init(&Preparing, 0, 0);
    sem_init(&ThirdPatient, 0, 0); 
    sem_init(&SecondPatient, 0, 0);
    sem_init(&AcceptOnebyOne, 0, 1);
    for (i = 0; i < max_HealthcareStaff; i++){
        pthread_create(&healthcareId[i], NULL, HealthcareStaff, (void *)&Number[i+1]);
    }
    

    // Create the Patients thread.
    for (i = 0; i < numPatientss; i++) {
        pthread_create(&PatientId[i], NULL, Patients, (void *)&Number[i+1]);
    }

    // Join each of the threads to wait for them to finish.
    for (i = 0; i < numPatientss; i++) {
        pthread_join(PatientId[i],NULL);
    }

    // When all of the Patients are finished, kill the Staff thread.

    allDone = 1;

    for (i = 0; i < max_HealthcareStaff; i++) {
        sem_post(&StaffStatu); // Wake the Staff so he will exit.
    }
    

    for (i = 0; i < max_HealthcareStaff; i++) {
        pthread_join(healthcareId[i], NULL);
    }
    system("PAUSE");   

    return 0;
}

void *Patients(void *number) {
     int num = *(int *)number;
     printf("Patients %d entering the hospital.\n", num);
     sem_wait(&waitingRoom); 
     printf("Patients %d entering the waiting room.\n", num);
     sem_wait(&testroom);//Number of patients transferred to 8 rooms where tests will be performed
     sem_wait(&AcceptOnebyOne); //It admits patients to the rooms one by one.
     usleep(10000);
     sem_post(&AcceptOnebyOne);
     sem_post(&waitingRoom);
     
	//The count variable is the variable that holds Max 3 people who must enter the rooms
     if(count == 0){
        printf("+ Patients %d waking the staff and filling the form.\n", num);
        sem_post(&StaffStatu); // Wait for the Staff to finish covid test
		count = count+1;
     }
	 else if(count == 1){
		 printf("+ Patients %d filling the form.\n", num);
		sem_post(&SecondPatient);
        count=count+1;
	 }
	  else if(count == 2){
		 printf("+ Patients %d filling the form.\n", num);
		sem_post(&ThirdPatient);
        count=0;
	 }
     sem_wait(&Preparing); // Give up the chair.
     usleep(10000);
     sem_post(&testroom);
     printf("- Patients %d leaving the hospital.\n", num);
     }

void *HealthcareStaff(void *number1){
    int healthcareStaffNum=*(int *)number1;
    while (!allDone) { // Sleep until someone arrives and wakes you..
    
    sem_wait(&StaffStatu); // Skip this stuff at the end...
    if (!allDone)
    {
	printf("[X][ ][ ]\n");
	printf("--The last 2 people,Please, pay attention to your social distance and hygiene; use a mask\n");
	sem_wait(&SecondPatient);
	printf("[X][X][ ]\n");
	printf("--The last people,Please, pay attention to your social distance and hygiene; use a mask\n");
	sem_wait(&ThirdPatient);
	printf("[X][X][X]\n");
        printf("Covid test unit %d 's medical staff apply the covid test\n",healthcareStaffNum);
 	printf("--------------------------------------------------------\n");
        TestTime(5);
	closeStatus++;
	for(int i = 0; i < 3; i++){
	     sem_post(&Preparing);
	}
	 printf("The HealthcareStaff is ventilating the ' %d ' Covid test room \n",healthcareStaffNum);
	 printf("                                                              \n");	
    }	  
	if(closeStatus==8){
 	 printf("The Covid 19 test unit's are closing.\n");
	 closeStatus=0;
	}  
   }
}
void TestTime(int secs) {
     int len = secs;
     sleep(len);
}
