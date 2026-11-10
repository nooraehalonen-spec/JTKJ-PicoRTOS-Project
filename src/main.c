
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 

//this buffer if for later traslation function (morse code to text)
#define INPUT_BUFFER_SIZE 256

//Add here necessary states
enum state { IDLE=1, INPUT, TRANSLATE };
enum state programState = IDLE;

//task that receives input from terminal and determines the state
/*
static void task_from_terminal(void *arg){
    (void)arg;
    while(1){
        if (programState == IDLE){
            int c = getchar_timeout_us(0);
            if (c == '\n'|| c == '\r'){
                programState = INPUT;
            }
            else if( c == 't'){
                programState = TRANSLATE;
            }
            //else if( c == 'somethingelse'){
            //    programState = somethingelse;
            //}
            else {
                vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
            }
        }
    }
}
*/ 

//IMU:n yksittäiset arvot
struct imu_data{
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float t;
} imu_number;

int spaces = 0;

static void button(uint which_button, uint32_t event_mask){

    //while(1){
    //if(programState == INPUT){

    //Tutkitaan kumpaa nappia painetaan, jos 1 tulkitaan asento ja siitä . tai -
    if (which_button == BUTTON1){
        if ((-0.9 >= imu_number.ay && imu_number.ay >= -1.1) || (0.9 <=imu_number.ay && imu_number.ay <= 1.1)){
                printf(".");
            }

            else if ((0.9 <= imu_number.az && imu_number.az <= 1.1) || (-1.1 <= imu_number.az && imu_number.az <= -0.9)){
                printf("-");
            }
        
        spaces = 0;
    }

    //Jos 2 nappi, tulostaa välilyönnin
    else{
        //Laskee välilyöntien määrän, jos 3, tulostaa rivinvaihtomerkin.
        spaces += 1;

        if (spaces < 3){
            printf(" ");
        }

        else{
            printf("\n");
            //programState = IDLE
        }
    }
    //}
    //}
}

static void example_task(void *arg)
{
    (void)arg;
    init_ICM42670();

    ICM42670_start_with_default_values();

 
    for (;;)
    {
        tight_loop_contents(); // Modify with application code here.

        //IMU tulkitsee asennot ja sijoittaa ne structiin
        ICM42670_read_sensor_data(&imu_number.ax, &imu_number.ay, &imu_number.az, &imu_number.gx, &imu_number.gy, &imu_number.gz, &imu_number.t);
        //printf("%f %f %f %f %f %f \n", imu_number.ax, imu_number.ay, imu_number.az, imu_number.gx, imu_number.gy, imu_number.gz);

        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main() {
    stdio_init_all();
    // Uncomment this lines if you want to wait till the serial monitor is connected
    /*while (!stdio_usb_connected()){
        sleep_ms(10);
    }*/ 
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.

    init_button1();

    init_button2();
        
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, &button);
    
    gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_FALL, true, &button);

    


    TaskHandle_t myExampleTask = NULL;
    // Create the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(example_task,       // (en) Task function
                "example",              // (en) Name of the task 
                DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,               // (en) Arguments of the task 
                2,                  // (en) Priority of this task
                &myExampleTask);    // (en) A handle to control the execution of this task

    if(result != pdPASS) {
        printf("Example Task creation failed\n");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}

