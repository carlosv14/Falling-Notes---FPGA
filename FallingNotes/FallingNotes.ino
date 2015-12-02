#include "VGA.h"
#include "SmallFS.h"
#include "cbuffer.h"
#include "Notes.h"

#define FOURTH_BUTTON FPGA_BTN_3
#define THIRD_BUTTON FPGA_BTN_2
#define SECOND_BUTTON FPGA_BTN_1
#define FIRST_BUTTON FPGA_BTN_0

#define SCORECHARS 4
#define SCOREX 123
#define SCOREY 0

int ifpos = 10;
int ifneg = 5;
int ifbonus= ifpos+ifneg;
int nivel = 1;
int errCounter = 0;
int passedLimit = 2;
unsigned long Index = 0;
unsigned long TimerCount = 0;
volatile unsigned char PlayingSound;

const char * poscomments[] = {"NICE!","GREAT!","+10!","GOOD!"};
const char * negcomments[] = {"OOPS!","-5!","WRONG!","BAD MOVE!"};
unsigned char scorearea[ 9*((8* SCORECHARS)+1) ];
int passed =0;
int delaytime=155;
SmallFSFile audiofile;

#define FRAME_SIZE  64
struct AudioData {
    unsigned char count;
    unsigned char sample[FRAME_SIZE];
};

CircularBuffer<AudioData, 2> audioBuffer;
AudioData currentData;

void DAC_SetOutput(unsigned char data)
{
    unsigned int level0 = ((unsigned int)data) << 8;
    unsigned int level1 = ((unsigned int)data) << 8;
    unsigned int level = (level1 << 16) | level0;    
    
    SIGMADELTADATA = level;
}                             

void SoundInit(void) 
{
    // Enable channel 0 and 1
    SIGMADELTACTL = 0x03;

    // Clear timer counter.
    TMR0CNT = 0;
 
    // Set up timer , no prescaler.
    TMR0CMP = 0;
    TMR0CTL = (0 << TCTLENA)| (1 << TCTLCCM)| (1 << TCTLDIR)| (1 << TCTLIEN);
 
    // Enable timer 0 interrupt on mask.
    INTRMASK |= (1 << INTRLINE_TIMER0);
 
    // Globally enable interrupts.
    INTRCTL = (1 << 0);
}

void SoundOff(void) 
{
    DAC_SetOutput(0);

    // Disable timer 0
    TMR0CTL &= ~(_BV(TCTLENA));
}

void AudioFillBuffer()
{
    int r;

    AudioData d;
    while (!audioBuffer.isFull()) {
        r = audiofile.read(&d.sample[0], FRAME_SIZE);
        if (r != 0) {
            d.count = r; // 1 sample per byte
            audioBuffer.push(d);
        } else {
            audiofile.seek(0, SEEK_SET);
        }
    }
}

void SoundPlay(const char *fileName) 
{    
    unsigned int sampleCount;
    unsigned int frameRate;
    
    audiofile = SmallFS.open(fileName);
    audiofile.read(&sampleCount, sizeof(unsigned int));
    audiofile.read(&frameRate, sizeof(unsigned int));
    
    Serial.print("Sample Count = ");
    Serial.println(sampleCount);

    Serial.print("Frame Rate = ");
    Serial.println(frameRate); 
    
    Index = 0;
    PlayingSound = 1;
    AudioFillBuffer();
    currentData = audioBuffer.pop();

    TMR0CNT = 0;
    TMR0CMP = (CLK_FREQ/frameRate) - 1;
    
    // Enable timer 0
    TMR0CTL |= 1 << TCTLENA;
}

void _zpu_interrupt ()
{
    if ( TMR0CTL & (1 << TCTLIF)) {
        unsigned char sample, hasData = 1;
        
        if (Index >= currentData.count) {
            if (audioBuffer.hasData()) {
                Index = 0;
                currentData = audioBuffer.pop();
            } else {
                hasData = 0;
            }
        }
        
        if (hasData) {
            sample = currentData.sample[Index];
            DAC_SetOutput(sample);
	    Index++;
	} else {
            SoundOff();
            PlayingSound = 0;
        }
        
        /* Clear the interrupt flag on timer register */
        TMR0CTL &= ~(1 << TCTLIF);
        TMR0CNT = 0;
    }
}

int hasComment=0;
boolean hasCar1=false;
boolean hasCar2=false;
boolean hasCar3=false;
boolean hasCar4=false;
unsigned char cval[4];
int note_width = 12;
int note_height = 10;
int contador;

Notes * Carril1[10];
Notes * Carril2[10];
Notes * Carril3[10];
Notes * Carril4[10];
int index1=0;
int index2=0;
int index3=0;
int index4=0;
 int score;
unsigned char readValues(){
        unsigned int s = 0;
        cval[0] = digitalRead( FIRST_BUTTON );
	cval[1] = digitalRead( SECOND_BUTTON );
	cval[2] = digitalRead( THIRD_BUTTON );
	cval[3] = digitalRead( FOURTH_BUTTON );
       if(cval[0])
         s = 1;
       else if(cval[1])
         s = 2;
       else if(cval[2])
         s = 3;
       else if(cval[3])
         s = 4;
       
      return s;
  
}
#ifndef __linux__
unsigned xrand() {
	CRC16APP=TIMERTSC;
	return CRC16ACC;
}
#else
#define xrand rand
#endif


unsigned char REDCOLOR[] = {
BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,BLACK,RED,RED,RED,RED,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,RED,RED,RED,RED,RED,RED,BLACK,BLACK,BLACK,
BLACK,BLACK,RED,RED,RED,RED,RED,RED,RED,RED,BLACK,BLACK,
BLACK,RED,RED,RED,RED,RED,WHITE,WHITE,RED,RED,RED,BLACK,
BLACK,RED,RED,RED,RED,RED,WHITE,WHITE,RED,RED,RED,BLACK,
BLACK,RED,RED,RED,RED,RED,RED,RED,RED,RED,RED,BLACK,
BLACK,RED,RED,RED,RED,RED,RED,RED,RED,RED,RED,BLACK,
BLACK,BLACK,RED,RED,RED,RED,RED,RED,RED,RED,BLACK,BLACK,
BLACK,BLACK,BLACK,RED,RED,RED,RED,RED,RED,BLACK,BLACK,BLACK
};

unsigned char GREENCOLOR[] = {
BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,BLACK,GREEN,GREEN,GREEN,GREEN,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,BLACK,BLACK,BLACK,
BLACK,BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,BLACK,BLACK,
BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,WHITE,WHITE,GREEN,GREEN,GREEN,BLACK,
BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,WHITE,WHITE,GREEN,GREEN,GREEN,BLACK,
BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,BLACK,
BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,BLACK,
BLACK,BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,BLACK,BLACK,
BLACK,BLACK,BLACK,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,BLACK,BLACK,BLACK
};

unsigned char BLUECOLOR[] = {
BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,BLACK,BLUE,BLUE,BLUE,BLUE,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLACK,BLACK,BLACK,
BLACK,BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLACK,BLACK,
BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,WHITE,WHITE,BLUE,BLUE,BLUE,BLACK,
BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,WHITE,WHITE,BLUE,BLUE,BLUE,BLACK,
BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLACK,
BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLACK,
BLACK,BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLACK,BLACK,
BLACK,BLACK,BLACK,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLACK,BLACK,BLACK
};

unsigned char PURPLECOLOR[] = {
BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,BLACK,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,BLACK,BLACK,BLACK,
BLACK,BLACK,BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,BLACK,BLACK,
BLACK,BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,BLACK,
BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,WHITE,WHITE,PURPLE,PURPLE,PURPLE,BLACK,
BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,WHITE,WHITE,PURPLE,PURPLE,PURPLE,BLACK,
BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,
BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,
BLACK,BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,BLACK,
BLACK,BLACK,BLACK,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,PURPLE,BLACK,BLACK,BLACK
};


unsigned char * colors[] = { GREENCOLOR,REDCOLOR,BLUECOLOR};
char types[] = { 'v','r','b'};
boolean valuefirst = true;

void addRandomNote()
{
	int i = 0;
         i= xrand() %4;
        int c =  xrand() %3;
    
      
        if(i==1 && index2<10 && !hasCar2){
          i= 40 + 12*i;
          Carril2[index2] = new Notes(i,10,note_width,note_height,colors[c],types[c]);
          index2++;
          hasCar2=true;
        }else if(i==2&& index3<10 && !hasCar3){
          i = 80 + 12;
           Carril3[index3] = new Notes(i,10,note_width,note_height,colors[c],types[c]);
          index3++;
          hasCar3=true;

        }else if(i==3&& index4<10 && !hasCar4){
          i = 120+12;
           Carril4[index4] = new Notes(i,10,note_width,note_height,colors[c],types[c]);
          index4++;
          hasCar4=true;
        }else if(i==0 && index1<10 &&!hasCar1){
          i=12; 
           Carril1[index1] = new Notes(i,10,note_width,note_height,colors[c],types[c]);
          index1++;
          hasCar1=true;
        }
      
}

int randomComment()
{
	return xrand() %4;
 
}


int  commentcarril = 0;
const char *ctype;

int comment(int carril, const char *type){
  int r = randomComment();
        if(type =="g"){
          if(carril ==1){
            VGA.setColor(GREEN);
            VGA.printtext(1,50,poscomments[r],true);
          }else if(carril ==2){
            VGA.setColor(GREEN);
            VGA.printtext(41,50,poscomments[r],true);
          }else if(carril ==3){
            VGA.setColor(GREEN);
            VGA.printtext(81,50,poscomments[r],true);
          }
      
        }
        else if(type =="b"){
          if(carril ==1){
            VGA.setColor(RED);
            VGA.printtext(1,50,negcomments[r],true);
          }else if(carril ==2){
            VGA.setColor(RED);
            VGA.printtext(41,50,negcomments[r],true);
          }else if(carril ==3){
            VGA.setColor(RED);
            VGA.printtext(81,50,negcomments[r],true);
          }
         
        }else if(type =="y"){
             if(carril ==1){
            VGA.setColor(BLUE);
            VGA.printtext(1,50,"BONO!",true);
          }else if(carril ==2){
            VGA.setColor(BLUE);
            VGA.printtext(41,50,"BONO!",true);
          }else if(carril ==3){
            VGA.setColor(BLUE);
            VGA.printtext(81,50,"BONO!",true);
          }
        }
	commentcarril = carril;
        ctype = type;
        hasComment=1;
        return r;
     
}
void removeComment(int pos, int carril,const char *type){
  if(type =="g"){ 
   if(carril ==1){
            VGA.setColor(BLACK);
            VGA.printtext(1,50,poscomments[pos],true);
          }else if(carril ==2){
            VGA.setColor(BLACK);
            VGA.printtext(41,50,poscomments[pos],true);
          }else if(carril ==3){
            VGA.setColor(BLACK);
            VGA.printtext(81,50,poscomments[pos],true);
          }
          
  }   else if(type =="b"){
          if(carril ==1){
              VGA.setColor(BLACK);
            VGA.printtext(1,50,negcomments[pos],true);
          }else if(carril ==2){
              VGA.setColor(BLACK);
            VGA.printtext(41,50,negcomments[pos],true);
          }else if(carril ==3){
              VGA.setColor(BLACK);
            VGA.printtext(81,50,negcomments[pos],true);
          }
       
        }else if(type =="y"){
             if(carril ==1){
            VGA.setColor(BLACK);
            VGA.printtext(1,50,"BONO!",true);
          }else if(carril ==2){
            VGA.setColor(BLACK);
            VGA.printtext(41,50,"BONO!",true);
          }else if(carril ==3){
            VGA.setColor(BLACK);
            VGA.printtext(81,50,"BONO!",true);
          }
        }
        hasComment = 0;
  
}
  int  commentpos = 0;

void drawX(){
 VGA.setColor(RED);
 if(errCounter==1)
 VGA.printtext(0,0,"X",true);
 else if(errCounter==2)
 VGA.printtext(6,0,"X",true);
 else if(errCounter==3){
 VGA.printtext(12,0,"X",true);
 if(contador%133){
 nivel++;
 if(nivel<=3)
  loop();
 }
 }
}
void handleMovement(unsigned char event){
   
if(index1>0){
  if(event==1 && Carril1[index1-1]->active == 1 && (Carril1[index1-1]->y>100 && Carril1[index1-1]->y<104)){
          Carril1[index1-1]->active =0;
          if(Carril1[index1-1]->type == 'v'){
          score+=ifpos;
           commentpos = comment(1,"g");
           Carril1[index1-1]->exploted = 1;
          }else if(Carril1[index1-1]->type == 'r'){
          if(score>5)
          score-=ifneg;
          errCounter++;
          drawX();
           commentpos = comment(1,"b");
          }else if(Carril1[index1-1]->type == 'b'){
          score+=ifbonus;
           commentpos = comment(1,"y");
          }
          update_score();
         
        
      }
      Carril1[index1-1]->drawPiece();
      Carril1[index1-1]->moveNote();
      if(Carril1[index1-1]->active==0){
        if(Carril1[index1-1]->type == 'v' &&  Carril1[index1-1]->exploted == 0)
            passed++;
           Carril1[index1-1]->deletePiece();
           delete Carril1[index1-1];     
           index1--;  
           hasCar1= false;
            VGA.setColor(160,82,45);
            VGA.drawLine(0,105,158,105);
      }
    }else if(index2>0){
      if(event==2 && Carril2[index2-1]->active == 1 && (Carril2[index2-1]->y>100 && Carril2[index2-1]->y<104)){
          Carril2[index2-1]->active =0;
          if(Carril2[index2-1]->type == 'v'){
          score+=ifpos;
           commentpos = comment(2,"g");
           Carril2[index2-1]->exploted = 1;

          }else if(Carril2[index2-1]->type == 'r'){
          if(score>5)
          score-=ifneg;
           errCounter++;
           drawX();
           commentpos = comment(2,"b");
          }else if(Carril2[index2-1]->type == 'b'){
          score+=ifbonus;
           commentpos = comment(2,"y");
          }
          update_score();
         
        
      }
      Carril2[index2-1]->drawPiece();
      Carril2[index2-1]->moveNote();
      if(Carril2[index2-1]->active==0){
        if(Carril2[index2-1]->type == 'v' && Carril2[index2-1]->exploted == 0)
          passed++;
           Carril2[index2-1]->deletePiece();
           delete Carril2[index2-1];     
           index2--; 
            hasCar2= false; 
            VGA.setColor(160,82,45);
            VGA.drawLine(0,105,158,105);
      }
    }else if(index3>0){
      if(event==3 && Carril3[index3-1]->active == 1 && (Carril3[index3-1]->y>100 && Carril3[index3-1]->y<104)){
          Carril3[index3-1]->active =0;
          if(Carril3[index3-1]->type == 'v'){
          score+=ifpos;
           commentpos = comment(3,"g");
           Carril3[index3-1]->exploted = 1;

          }else if(Carril3[index3-1]->type == 'r'){
          if(score>5)
          score-=ifneg;
           errCounter++;
           drawX();
           commentpos = comment(3,"b");
          }else if(Carril3[index3-1]->type == 'b'){
          score+=ifbonus;
           commentpos = comment(3,"y");
          }
          update_score();
         
        
      }
      Carril3[index3-1]->drawPiece();
      Carril3[index3-1]->moveNote();
      if(Carril3[index3-1]->active==0){
        if(Carril3[index3-1]->type == 'v' &&Carril3[index3-1]->exploted ==0 )
            passed++;
           Carril3[index3-1]->deletePiece();
           delete Carril3[index3-1];     
           index3--; 
            hasCar3= false; 
            VGA.setColor(160,82,45);
            VGA.drawLine(0,105,158,105);
      }
    }else if(index4>0){
      
      
    if(event==4 && Carril4[index4-1]->active == 1 && (Carril4[index4-1]->y>100 && Carril4[index4-1]->y<104)){
          Carril4[index4-1]->active =0;
          if(Carril4[index4-1]->type == 'v'){
          score+=ifpos;
           commentpos = comment(4,"g");
           Carril4[index4-1]->exploted = 1;
          }else if(Carril4[index4-1]->type == 'r'){
          if(score>5)
          score-=ifneg;
           errCounter++;
           drawX();
           commentpos = comment(4,"b");
          }else if(Carril4[index4-1]->type == 'b'){
          score+=ifbonus;
           commentpos = comment(4,"y");
          }
          update_score();
         
        
      }
      Carril4[index4-1]->drawPiece();
      Carril4[index4-1]->moveNote();
      if(Carril4[index4-1]->active==0){
        if(Carril4[index4-1]->type == 'v' &&  Carril4[index4-1]->exploted == 0)
            passed++;
           Carril4[index4-1]->deletePiece();
           delete Carril4[index4-1];     
           index4--;  
           hasCar4= false;
            VGA.setColor(160,82,45);
            VGA.drawLine(0,105,158,105);
      }
    }
    
  if(passed == passedLimit){
    errCounter++;
    Serial.println(passed);
    passed = 0;
    drawX();
  }
    
  
  
    delay(10);
    
  
}


char *sprintint(char *dest, int max, unsigned v)
{
	dest+=max;
	*dest--='\0';

	do {
		*dest = (v%10)+'0';
		dest--, max--;
		if (max==0)
			return dest;
		v=v/10;
	} while (v!=0);
	return dest+1;
}

void update_score()
{
	char tscore[SCORECHARS+1];
	char *ts = tscore;
	int x = SCOREX;

	int i;
	for (i=0;i<6;i++)
		tscore[i]=' ';

    tscore[6]='\0';

	VGA.writeArea( SCOREX, SCOREY, (8*SCORECHARS)+1,8+1, scorearea);

	sprintint(tscore, sizeof(tscore)-1, score);

	while (*ts==' ') {
		ts++;
		x+=8;
	}
	VGA.setColor(BLACK);
	VGA.printtext(x,SCOREY,ts,true);

	VGA.setColor(YELLOW);
	VGA.printtext(x+1,SCOREY+1,ts,true);
}

void score_init()
{
	/* Save the score area */
	VGA.readArea( SCOREX, SCOREY, (8*SCORECHARS)+1,8+1, scorearea);
}

void score_draw()
{
        VGA.setColor(BLACK);
	VGA.printtext(50,0,"END",true);
	VGA.setColor(BLACK);
	VGA.printtext(80, 0, "Score:", true);
	VGA.setColor(YELLOW);
	VGA.printtext(80, 0, "Score:", true);
        update_score();

}



void drawBoard(){
  VGA.setColor(160,82,45);
  VGA.drawLine(40,0,40,120);
  VGA.drawLine(80,0,80,120);
  VGA.drawLine(120,0,120,120);
  VGA.drawLine(0,105,158,105);
}


void prepareLevel(const char * soundfile){
 SoundPlay(soundfile);
  drawBoard();
  score_init();
  score = 0;
  contador = 0;
  score_draw();
}
void setup(){
  
  VGA.begin(VGAWISHBONESLOT(9),CHARMAPWISHBONESLOT(10));
  VGA.clear();
  Serial.begin(115200);
   SoundInit();
  Serial.println("Starting");
   pinMode(FPGA_J2_6, OUTPUT);
    pinModePPS(FPGA_J2_6, HIGH);
    outputPinForFunction(FPGA_J2_6, 0);
    
    pinMode(FPGA_J2_7, OUTPUT);
    pinModePPS(FPGA_J2_7, HIGH);
    outputPinForFunction(FPGA_J2_7, 0);
     if (SmallFS.begin()<0) {
        Serial.println("No SmalLFS found, aborting.");
        
        while(1) {};
    }
    
 
}

void changeLevel(int level){
  VGA.clear();
  for(int i = 0; i<10;i++){
     delete Carril1[i]; 
     delete Carril2[i]; 
     delete Carril3[i]; 
     delete Carril4[i]; 
  }
  index1= 0;
  index2 = 0;
  index3 =0;
  index4 = 0;
  errCounter= 0;
  passed = 0;
  if(nivel==1)
  prepareLevel("sax.snd");
  if(nivel==2)
  prepareLevel("crazy.snd");
}

void level(int modtime){
  while(true){
     contador++;
if (PlayingSound) {
        AudioFillBuffer();
    }

 if(contador%modtime==0){ 
      addRandomNote();
 }

 
if(nivel==1){
      if(contador==2750){
           nivel = 2;
           break;
      }else if(contador==2635){
           
       VGA.setColor(BLACK);
    	VGA.printtext(50,0,"2",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"END",true);	
     }else if(contador==2480){
       	VGA.setColor(BLACK);
    	VGA.printtext(50,0,"3",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"2",true);
     }else if(contador==2325){
            VGA.setColor(BLACK);
    	VGA.printtext(50,0,"4",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"3",true);
     }else if(contador==2170){
           VGA.setColor(BLACK);
    	VGA.printtext(50,0,"5",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"4",true);
     }else if(contador==2015){
            VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"5",true);
       }
 }else if(nivel==2){
    if(contador==4080){
           nivel = 3;
           break;
      }else if(contador==4000){
           
            VGA.setColor(BLACK);
    	VGA.printtext(50,0,"2",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"END",true);	
     }else if(contador==3900){
       	VGA.setColor(BLACK);
    	VGA.printtext(50,0,"3",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"2",true);
     }else if(contador==3800){
            VGA.setColor(BLACK);
    	VGA.printtext(50,0,"4",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"3",true);
     }else if(contador==3700){
           VGA.setColor(BLACK);
    	VGA.printtext(50,0,"5",true);
    	VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"4",true);
     }else if(contador==3600){
            VGA.setColor(YELLOW);
    	VGA.printtext(50,0,"5",true);
       }
   
   
 }
 
  if(hasComment && (contador%modtime==0 )&& commentcarril!=0)
      removeComment(commentpos,commentcarril,ctype);

  handleMovement(readValues());
    
  }
  
}

void waitForStart(){
  int event = 0;
  VGA.setColor(YELLOW);
  while(event!=1){
    event  = readValues();
  VGA.printtext(40,50,"TO START",true);
  VGA.printtext(20,60,"PRESS BUTTON 1",true);
   
  }
}
void readySetGo(){
 int numero  = 0;
 int veces = 0;
    VGA.clear();
    VGA.setColor(YELLOW);
  while(true){
   if(numero%157==0  && veces == 0){
       VGA.printtext(50,40,"READY!",true);
       veces++;
   }else if(numero%157==0  && veces == 1){
     VGA.printtext(50,60,"SET!",true);
       veces++;
   }else if(numero%157==0  && veces == 2){
     VGA.printtext(50,80,"GO!",true);
       veces++;
   }
   numero++;
   if(veces==3 && numero%157==0)
     break;
     delay(15);
  } 
  
}
void loop()
{
  VGA.clear();
  waitForStart();
  readySetGo();
  if(nivel==2)
    delaytime=100;
  changeLevel(nivel);
  level(delaytime);

}
