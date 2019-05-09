#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_SI5351.h>

Adafruit_SI5351 clockgen = Adafruit_SI5351();

class EEPromRW{
 private:
  union bilf{
    byte b[4];
    int i[2];
    long li;
    float f;
  };
  void EWriteBytes(int addr, byte size, bilf b){
    switch(size){
    default:
      EEPROM.write(addr+3,b.b[3]);
    case 3:
      EEPROM.write(addr+2,b.b[2]);
    case 2:
      EEPROM.write(addr+1,b.b[1]);
    case 1:
      EEPROM.write(addr+0,b.b[0]);
    case 0:
      break;
    }
  }
  bilf EReadBytes(int addr, byte size){
    bilf b;
    b.li=0;
    switch (size){
    default:
      b.b[3]=EEPROM.read(addr+3);
    case 3:
      b.b[2]=EEPROM.read(addr+2);
    case 2:
      b.b[1]=EEPROM.read(addr+1);
    case 1:
      b.b[0]=EEPROM.read(addr+0);
    case 0:
      break;
    }
    return(b);
  }
  void WriteByte(int addr, byte b){
    bilf bilf;
    bilf.b[0]=b;
    EWriteBytes(addr,1,bilf);
  }
  byte ReadByte(int addr){
    bilf bilf;
    bilf=EReadBytes(addr,1);
    return(bilf.b[0]);
  }
  void WriteInt(int addr, int i){
    bilf bilf;
    bilf.i[0]=i;
    EWriteBytes(addr,2,bilf);
  }
  int ReadInt(int addr){
    bilf bilf;
    bilf=EReadBytes(addr,2);
    return(bilf.i[0]);
  }
  void WriteLong(int addr, long li){
    bilf bilf;
    bilf.li=li;
    EWriteBytes(addr,4,bilf);
  }
  long ReadLong(int addr){
    bilf bilf;
    bilf=EReadBytes(addr,4);
    return(bilf.li);
  }
  void WriteFloat(int addr, float f){
    bilf bilf;
    bilf.f=f;
    EWriteBytes(addr,4,bilf);
  }
  float ReadFloat(int addr){
    bilf bilf;
    bilf=EReadBytes(addr,4);
    return(bilf.f);
  }
 public:
  void Write_fa(int i){
    WriteFloat(0,i);
  }
  int Read_fa(){
    return(ReadFloat(0));
  }
  void Write_fb(int i){
    WriteFloat(4,i);
  }
  int Read_fb(){
    return(ReadFloat(4));
  }
  void Write_fc(int i){
    WriteFloat(8,i);
  }
  int Read_fc(){
    return(ReadFloat(8));
  }
  /*
  void WriteByte(byte b){
    WriteByte(0,b);
  }
  byte ReadByte(){
    return(ReadByte(0));
  }
  void WriteInt(int i){
    WriteInt(1,i);
  }
  int ReadInt(){
    return(ReadInt(1));
  }
  void WriteLong(long li){
    WriteLong(3,li);
  }
  long ReadLong(){
    return(ReadLong(3));
  }
  void WriteFloat(float f){
    WriteFloat(7,f);
  }
  float ReadFloat(){
    return(ReadFloat(7));
    }*/
  EEPromRW(){}
  ~EEPromRW(){}
}EEPromRW;

class SerialIO{ // code re-used from S2334
 private:
#define BUFF_LENGTH 20
#define CHAR_LF 10
#define CHAR_CR 13
  char cbuf[BUFF_LENGTH];
  char NewString[BUFF_LENGTH];
  int NewStringLength;
  int cptr=0;
  byte AppendBuffer(){
    byte c;
    while(Serial.available()){
      c=Serial.read();
      if(c!=CHAR_LF){ // ignore LF
	if(cptr<BUFF_LENGTH)cbuf[cptr++]=c;
	if(cptr>=BUFF_LENGTH){ // response to buffer overflow
	  if(c==CHAR_CR){
	    cptr=0;
	    return(0);
	  } else {
	    return(-1);
	  }
	} else {
	  if(c==CHAR_CR){
	    cbuf[cptr-1]=0;
	    strcpy(NewString,cbuf);
	    NewStringLength=cptr-1;
	    cptr=0;
	    return(NewStringLength);
	  }
	}
      }
    }
    return(0);
  }
  // check command string match
  bool CheckQueryString(char * QueryString,bool exactmatch){
    int lenQuery;
    lenQuery=strlen(QueryString);
    if(exactmatch){
      if(lenQuery==NewStringLength)return(0==strncmp(NewString,QueryString,lenQuery));
    } else {
      if(lenQuery<NewStringLength)return(0==strncmp(NewString,QueryString,lenQuery));
    }
    return(false);
  }
 public:
  Init(){
    Serial.begin(9600);
    do{
      delay(200);
    } while (!Serial); 
  }
  // return length of new string once CR received
  bool CheckSerialIn(){
    return(AppendBuffer()>0);
  }
  void GetSerialInString(char *x){
    strcpy(x,NewString);
  }
  char GetStringChar0(){
    return(NewString[0]);
  }
  // check new serial input matches query string
  bool CheckMatch(char * QueryString){
    return(CheckQueryString(QueryString,true));
  }
  // check for serial input with floating point number
  bool CheckMatch(char * QueryString,float *mmm){
    char * ptrEnd;
    int lenQuery;
    lenQuery=strlen(QueryString);
    if(CheckQueryString(QueryString,false)){
      *mmm=strtod(NewString+lenQuery,&ptrEnd);
      return(ptrEnd==(NewString+NewStringLength));
    }
    return(false);
  }
  // check for serial input with integer base 10
  bool CheckMatch(char * QueryString,long *nnn){
    char * ptrEnd;
    int lenQuery;
    lenQuery=strlen(QueryString);
    if(CheckQueryString(QueryString,false)){
      *nnn=strtol(NewString+lenQuery,&ptrEnd,10);
      return(ptrEnd==(NewString+NewStringLength));
    }
    return(false);
  }
  // check for serial input with integer base 16 (hex numbers)
  bool CheckMatchX(char * QueryString,long *nnn){
    char * ptrEnd;
    int lenQuery;
    lenQuery=strlen(QueryString);
    if(CheckQueryString(QueryString,false)){
      *nnn=strtol(NewString+lenQuery,&ptrEnd,16);
      return(ptrEnd==(NewString+NewStringLength));
    }
    return(false);
  }
  void PrintString(char * zz){
    Serial.print(zz);
  }
  SerialIO(){}
  ~SerialIO(){}
}SerialIO;

char zz[80];
void ShowError(){
  SerialIO.PrintString("Error\n");
}
#define XTAL 25
float SetupA_PLL(float freq,int div,int frac){
  float a,x,n=0;
  a=freq*(float)div/XTAL;
  if(a<20){
    a=20;
    clockgen.setupPLLInt(SI5351_PLL_A,(int)a);
  } else if (a>50){
    a=50;
    clockgen.setupPLLInt(SI5351_PLL_A,(int)a);
  } else{
    n=a-(int)a;
    n*=frac;
    clockgen.setupPLL(SI5351_PLL_A, int(a),int(n),frac);
  }
  clockgen.setupMultisynthInt(0, SI5351_PLL_A, div);
  sprintf(zz,"DIV%d a=%d + %d /%d\n",div,int(a),int(n),frac);
  SerialIO.PrintString(zz);
  x  = int(a);
  n=(int)n;
  x += n/(float)frac;
  return(x*XTAL/div);
}

float SetA(float freq){
  float f,a;
  a=freq*20/XTAL;
  if(a<50){
    f=SetupA_PLL(freq,20,25);
  } else {
    a=freq*15/XTAL;
    if(a<50){
      f=SetupA_PLL(freq,15,33);
    } else {
      a=freq*10/XTAL;
      if(a<50){
	f=SetupA_PLL(freq,10,20);
      } else {
	a=freq*8/XTAL;
	if(a<50){
	  f=SetupA_PLL(freq,8,25);
	} else {
	  f=SetupA_PLL(freq,6,50);
	}
      }
    }
  }
  return(f);
}
float SetB(float freq){
  return(freq);
}

void ShowFA(float freq){
  float f;
  EEPromRW.Write_fa(f=SetA(freq));
  Serial.print("FrequencyA=");
  Serial.print(f);
  Serial.println("MHz");
}
void ShowFA(){
  ShowFA(EEPromRW.Read_fa());
}
void ShowFB(float freq){
  float f;
  EEPromRW.Write_fb(f=SetB(freq));
  Serial.print("FrequencyB=");
  Serial.print(f);
  Serial.println("MHz");
}
void ShowFB(){
  ShowFB(EEPromRW.Read_fb());
}

void ShowHelp(){
  SerialIO.PrintString("\nThis program controls SI5351 DDS frequency.  (dmc - 2019-05-09)\n");
  SerialIO.PrintString("faNNN.NN<cr> where NNN is the frequency.\n");
  SerialIO.PrintString("fbNNN.NN<cr> where NNN is the frequency.\n");
}

void setup() {
  SerialIO.Init();
  clockgen.begin();
  clockgen.enableOutputs(true);
}

void loop() {
  float freq;
  ShowFA();
  //  ShowFB();

  do{
    if(SerialIO.CheckSerialIn()){
      switch(SerialIO.GetStringChar0()){
      case 'f':
	if(SerialIO.CheckMatch("fa",&freq)){
	  ShowFA(freq);
	} else if(SerialIO.CheckMatch("fb",&freq)){
	  ShowFB(freq);
	} else {
	  ShowError();
	}
	break;
      default:
	ShowHelp();
        ShowFA();
        ShowFB();
	break;
      }
    }
  }while(true);
}
