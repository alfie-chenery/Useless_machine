//----------definitions----------
//install modules
#include <Servo.h>
#include <EEPROM.h>

//define global objects & variables
Servo aux_power;
Servo roof_arm;
Servo main_arm;
int NUM_OF_MODES = 14;
int UNUSED_PIN = 8;
int EEPROM_ADDRESS = 3; //manually += 1 if EEPROM cells die
/*
 * EEPROM has a rated lifespan of 10000 write cycles per individual cell.
 * As explained in chooseMode() each cell is written on average every 4th execution
 * So after 40000 executions the EEPROM_ADDRESS needs incrementing.
 * If this value reaches 128, the whole EEPROM is dead and chip needs replacing
 */

//define function headers with optional parameter
  //if function is called without optional wait parameter then wait will default to 0
void set_aux_power(int pos, int wait=0);
void set_roof_arm(int pos, int wait=0);
void set_main_arm(int pos, int wait=0);
  
//----------main code----------
void setup() {

  //seed random generator and choose mode
  int mode = chooseMode();
  Serial.println(mode);

    
  //attatch servos to pwm pins
  aux_power.attach(6);
  roof_arm.attach(11);
  main_arm.attach(10);

  homeServos();
  set_aux_power(100,500); //presses microswitch
                                              
  runProcedure(mode); //transition code written in function to keep setup readable
                      //main_arm and roof_arm should be homed inside of runProcedure

  homeServos(); //releases microswitch, cuts power

  /*
   * power to the arduino will likely be cut before the servo reaches its full right position
   * this is not an issue as it will be homed to centre on next startup
   * for the same reason, the below code will only run when power cannot be cut, such as
   * when being powered from USB during recoding.
  */

  delay(1000);
  aux_power.detach();           //detatch servos from pins
  roof_arm.detach();
  main_arm.detach();
  pinMode(LED_BUILTIN, OUTPUT); //initialise onboard LED for output, to be used in loop
  
}

void loop() {
  /*  
   *  code below flashes the onboard LED to signal normal execution should have ended
  */
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}


/*
 * servo set functions take parameter as integer from 0 to 100
 * servo limits are defined such that min is home position and max
 * is fully extended to perorm transitions
 * As such it may be that min > max if servo is mounted in reverse
 * 
 * set_main_arm(80) is touching the switch but not flipping it to have 
 * enough force to flip switch must go from 80 (or lower) straight to 100
 */
void set_aux_power(int pos, int wait){
  //define servo limits
  int Min = 1650;  //released
  int Max = 1450;  //pressed
  int sig = map(pos, 0, 100, Min, Max);
  aux_power.writeMicroseconds(sig);
  delay(wait);
}

void set_roof_arm(int pos, int wait){
  //define servo limits
  int Min = 1500;  //closed
  int Max = 950;  //open
  int sig = map(pos, 0, 100, Min, Max);
  roof_arm.writeMicroseconds(sig);
  delay(wait);
}

void set_main_arm(int pos, int wait){
  //define servo limits
  int Min = 2300;  //retracted
  int Max = 850;  //extended
  int sig = map(pos, 0, 100, Min, Max);
  main_arm.writeMicroseconds(sig);
  delay(wait);
}

void homeServos(){
  set_main_arm(0);
  set_roof_arm(0);
  set_aux_power(0);
}


void runProcedure(int mode){
  switch(mode){
    case 0: //immediate switch
      set_roof_arm(100, 250);
      set_main_arm(100, 750);
      set_main_arm(0, 750);
      set_roof_arm(0, 100);
      break;

    case 1: //pause before switching
      set_roof_arm(100, 250);
      set_main_arm(90, 2000);
      set_main_arm(100, 500);
      set_main_arm(0, 500);
      set_roof_arm(0, 100);
      break;

    case 2: //bang roof
      set_roof_arm(100,1000);
      set_roof_arm(0, 250);
      for(int i=0; i<3; i++){
        set_roof_arm(80, 250);
        set_roof_arm(0, 250);
      }
      set_roof_arm(100,500);
      set_main_arm(100,1000);
      set_main_arm(0,500);
      set_roof_arm(0,500);
      break;

    case 3: //slowly aproach switch, fast retraction
      for(int i=0; i<100; i++){
        set_roof_arm(i, 10);
      }
      delay(1000);
      for(int i=0; i<85; i++){
        set_main_arm(i, 10);
      }
      delay(2500);
      set_main_arm(100,150);
      set_main_arm(0,250);
      set_roof_arm(0,100);
      break;

    case 4: //slowly aproach switch, slow retraction
      for(int i=0; i<=100; i++){
        set_roof_arm(i, 10);
      }
      delay(1000);
      for(int i=0; i<=85; i++){
        set_main_arm(i, 10);
      }
      delay(2500);
      set_main_arm(100,150);
      delay(2000);
      for(int i=100; i>=0; i--){
        set_main_arm(i, 10);
      }
      delay(1000);
      for(int i=100; i>=0; i--){
        set_roof_arm(i, 10);
      }
      break;

    case 5: //repeatedly get closer to switch
      set_roof_arm(100, 1000);
      for(int i=0; i<5; i++){
        set_main_arm(60 + 5*i, 300); 
        set_main_arm(0, 500);
      }
      set_main_arm(100, 500);
      set_main_arm(0, 1000);
      set_roof_arm(0, 100);
      break;

    case 6: //shake arm after switch
      set_roof_arm(100, 1000);
      set_main_arm(100, 3000);
      for(int i=0; i<4; i++){
        set_main_arm(55, 100);
        set_main_arm(95, 100);
      }
      delay(500);
      set_main_arm(0, 300);
      set_roof_arm(0, 100);
      break;

    case 7: //look back after switching
      set_roof_arm(100, 250);
      set_main_arm(100, 1000);
      set_main_arm(0, 1000);
      set_roof_arm(0, 2000);
      set_roof_arm(100, 1500);
      set_roof_arm(0, 100);
      break;

    case 8: //peak before switching
      set_roof_arm(50, 1000);
      set_roof_arm(0, 2000);
      set_roof_arm(100, 150);
      set_main_arm(100, 350);
      set_main_arm(0, 250);
      set_roof_arm(0, 100);
      break;

    case 9: //trap arm in roof
      set_roof_arm(100, 250);
      set_main_arm(100, 1000);
      set_roof_arm(0, 500);
      set_roof_arm(100, 250);
      set_main_arm(0, 500);
      set_roof_arm(0, 100);
      break;

    case 10: //hold switch
      set_roof_arm(100, 1000);
      set_main_arm(100, 4500);
      set_main_arm(0, 500);
      set_roof_arm(0, 100);
      break;

    case 11: //speedrun the switch
      set_roof_arm(100, 150);
      set_main_arm(100, 350);
      set_main_arm(0, 2000);
      set_roof_arm(0, 100);
      break;

    case 12: //keep checking switch
      set_roof_arm(100, 200);
      set_main_arm(100, 500);
      for(int i=100; i>=95; i--){
        set_main_arm(i, 20);
      }

      for(int k=3; k>=0; k--){
        if(k != 3){
          set_main_arm(95, 300);
        }
        for(int i=95; i>=20*k; i--){
          set_main_arm(i, 20);
        }
      }

      set_main_arm(0,500);
      for(int i=100; i>=0; i--){
        set_roof_arm(i, 10);
      }
      break;
      

    case 13: //miss switch first time
      set_roof_arm(100, 250);
      set_main_arm(90, 750);
      set_main_arm(0, 750);
      set_roof_arm(0, 1500);
      set_roof_arm(100, 750);
      set_main_arm(100, 500);
      set_main_arm(0, 750);
      set_roof_arm(0, 100);
      break;
  }
}

int chooseMode(){
   
  int seed = analogRead(UNUSED_PIN); //seed random number generator with noise of unused pin
  randomSeed(seed);

  Serial.begin(9600);
   
  int lastRuns[4];
  for(int i=0; i<4; i++){
    lastRuns[i] = EEPROM.read(EEPROM_ADDRESS * 8 + i);
    Serial.print(lastRuns[i]);Serial.print(" ");
  }
  Serial.println("");
   
  int indexFlags[4];
  for(int i=0; i<4; i++){
    indexFlags[i] = EEPROM.read(EEPROM_ADDRESS * 8 + 4 + i);
    Serial.print(indexFlags[i]);Serial.print(" ");
  }
  Serial.println("");
  
  int choice = lastRuns[0]; //force at least one iteration
  while (choice == lastRuns[0] || choice == lastRuns[1] || choice == lastRuns[2] || choice == lastRuns[3]){
    choice = random(0,NUM_OF_MODES);
  }

  //use indexFlags to work out which cell to write the new value to
  int val = indexFlags[0];
  int i=0;
  while(indexFlags[i] == val){
    i++;
    if(i == 4){
      i=0;
      val = 1-val; //swap 0->1, 1->0
      break;
    }
  }
  //i now stores the index of the head of the list
  //val now stores 1 - old value of indexFlags[i]
  // ie the value index i should be set to now the list is updated
  
  EEPROM.write(EEPROM_ADDRESS * 8 + i, choice);
  EEPROM.write(EEPROM_ADDRESS * 8 + 4 + i, val); 
  
  return choice;
}

/*
 * EEPROM uses a more complicated method to be more efficient. We store the codes of the last 4
 * chosen modes and want to avoid picking these again. However when we pick a new one we want to
 * add this to the list in EEPROM and evict the oldest of the 4 currently in EEPROM.
 * Keeping the list in order and shifting all codes along causes 4 writes per execution so is
 * ineficient given EEPROM life expectancy. A single cell to track the head of the list would
 * need writing every execution while the actual bytes would be written every 4 executions (average)
 * so cells dying at different times would create very complex controlling code.
 * The solution used is to have 4 shadow bytes which are set to 0 or 1. The configuration of these
 * values can be used to deterine the head of the list.
 * 
 * The head of the list is the byte corresponding to the first shadow byte which is not equal to
 * shadow byte 0. If all shadow bytes are the same, then shadow byte 0 is the head of the list.
 * 
 * The list is stored in 8 consecutive bytes of EEPROM. The first 4 bytes store the list data
 * The next 4 bytes store the corresponding shadow bytes. EEPROM[i+4] is the shadow byte to EEPROM[i].
 * The offset of the list in EEPROM can be incremented when cells die after 40000 executions.
 * 
 * EXAMPLE RUNTHROUGH
 * [0,1,2,3] [0,0,0,0] From the start configuration all shadow bytes are 0, so the head is at 0
 * When we update index 0 with the chosen mode we set its shadow byte to 1 to mark it has been recently changed
 * [6,1,2,3] [1,0,0,0]
 * Now we see a 1 as the first shadow bit, so find the first occurance with a 0 shadow bit, and this is the head
 * [6,7,2,3] [1,1,0,0]
 * this continues
 * [6,7,8,3] [1,1,1,0]
 * [6,7,8,9] [1,1,1,1]
 * Now when checking for a 0 shadow bit there are none. Therefore we start again from the 0th cell, but this time
 * reset shadow bits to 0 to mark instead.
 * [4,7,8,9] [0,1,1,1]
 * Now we see the first shadow bit is 0 and so the first 1 occurance is the head of the list
 * [4,2,8,9] [0,0,1,1]
 * [4,2,0,9] [0,0,0,1]
 * [4,2,0,3] [0,0,0,0] ...
 * 
 * This method halves the capacity of EEPROM from 1024 bytes to 512 as each byte also uses a shadow byte
 * However, each individual cell is only written to once in every 4 executions (along with its shadow byte)
 * Therefore the code can survive 40000 executions before the EEPROM addresses need incrementing.
 * So overall we have doubled the lifespan of the chip by quadrupling efficiency and only having capacity.
 * If we stored more than the last 4 modes the efficiency would increase further.
 */

/*
 * Old method which does not use EEPROM
 * Uses multiple reads of noise pin to try to decrease the chance of 2 consecutive executions
 * picking the same mode. There is no formal proof this method helps t reduce runs, although
 * by inspection it seems to. However is inferior to the guarenteed effectiveness of the EEPROM
 * method. This method's only advantage is that it can work on chips with dead EEPROM.
 * 
 * 
int chooseMode(){
     //seed pseudo random number generator. Just using analogRead was producing runs
     //this method makes the value used as seed less likely to be the same as the 
     //last runthrough
     
    int seed = 0;
    for(int i=0; i<10; i++){
      int read = analogRead(UNUSED_PIN);
      seed += i * read;
      delay(50);
      //configure wait time, reducing the time means reads are closer together and more likely
      // to read the same noise value. Increasing the time means the box does nothing for longer
      // when it is first turned on while it chooses a seed value.
    }
    Serial.println(seed);
    randomSeed(seed);

    int mode = random(0,NUM_OF_MODES);   //generate random number in range 0 to number of modes
                                     //random is bound exclusive so numOfModes is total modes including mode 0
    return mode;
}
*/

//----------functions for testing----------
void printEEPROM(){
  Serial.begin(9600);
  
  for(int j=0; j<128; j++){
    Serial.print(j*8);
    Serial.print(": ");
    for(int i=0; i<8; i++){
      Serial.print(EEPROM.read(i + j*8), HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
  Serial.println("");
}

void resetEEPROM(){
  for(int i=0; i<4; i++){
    EEPROM.write(EEPROM_ADDRESS * 8 + i, i);
  }
  for(int i=0; i<4; i++){
    EEPROM.write(EEPROM_ADDRESS * 8 + 4 + i, 0);
  }
}
