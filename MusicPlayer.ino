
#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h>

//and the MP3 Shield Library
#include <SFEMP3Shield.h>
SdFat sd;
int songID;
int ofs;
int volume1, volume2;
SFEMP3Shield MP3;

/*
 * Free pin : 10~, 5~, 4, 3~, 1, 0
 */

void setup() {
    Serial.begin(115200);
    if(!sd.begin(SD_SEL, SPI_FULL_SPEED)) sd.initErrorHalt();
    if(!sd.chdir("/mp3/")) sd.errorHalt("sd.chdir");
   
    loadSeting();
    printInfo();
    MP3.begin();
    setupVolume(volume1, volume2);
    pinMode(A0, INPUT);
}

void printInfo()
{
  Serial.println("Mp3 Player - 19/06/2016 07:34");
  printSID();
  printV();
  
}

void printSID()
{
  Serial.print("Current mp3 ID :");
  Serial.println(songID);
}




void printV()
{
  Serial.print("Volume (L/R): ");
  Serial.print(volume1);
  Serial.print(" ");
  Serial.println(volume2);
}
char filename[32];
void printFn()
{
  Serial.print("Playing :");
  Serial.print(filename);
}


char temp[32];
void copy(char * a, char * b, int c)
{
    for (int i = 0; i< c; ++i)
      b[i] = a[i];
}
//songID 2 byte
// volume1 2byte
//volume2 2byte
//filename *n* 32 byte

void setupVolume(int v1, int v2)
{
   
  if (v1<0 ||v2<0 ||v1>100 ||v2>100)
  {
    v2 = v1 = 45;
  }
  if (v1<0) v1 = 0;
  if (v1> 90) v1 =90;
  if (v2<0) v2 = 0;
  if (v2> 90) v2 =90;
 
  volume1 = v1; 
  volume2 = v2;
  
  MP3.setVolume(v1, v2);
  printV();
}

void loadSeting()
{
    songID = 0;
    ofs = 0;
    volume1 = 0;
    volume2 =  0;
    SdFile f;
    while (f.openNext(sd.vwd(), O_READ))
    {
        f.getFilename(filename);
        f.close(); 
        Serial.println(filename);
        //if (false)
        if (isFnMusic(filename))
        {
        SdFile d;
        if (d.open("/mp3/dir.bin", O_WRITE|O_READ))
         songID = 0;
         d.read(&songID, 2);
         d.read(&volume1, 2);
         d.read(&volume2, 2);
         for (int i = 0; i< ofs; i++)
            d.read(temp, 32); 
        d.write(filename, 32);
        d.close();
        ofs++;
        }
          f.close();    
    }
   
}

void FnID(int i, char * bf, bool writeDown = false)
{
  SdFile d;
  d.open("/mp3/dir.bin", O_WRITE|O_READ);
  if (writeDown) d.write(&i, 2);
  else
  d.read(temp, 2);
  d.write(&volume1, 2);
  d.write(&volume2, 2);
  for (int i = 0; i< songID; i++)
        d.read(temp, 32); 
  d.read(bf, 32);
  d.close();
}

int chooseSID(int id)
{
  if (MP3.isPlaying()) MP3.stopTrack();
  songID = id;
  while (songID<0) songID += ofs;
  while (songID>= ofs) songID -= ofs;
  FnID(songID, filename, true);
  Serial.print("Now play :");
  Serial.println(filename);
  return MP3.playMP3(filename);
}
int  nextMusic()
{
  return chooseSID(songID+1);
}



bool isIdent(char c)
{
  return (c>='a' && c<='z') ||(c>='0' && c<= '9');
}


bool readInt(int & x)
{
  char c;
  x = 0;
  while(Serial.available())
  {
      c = Serial.read();
      if (c>='0' && c<='9')
      {
        x = x*10+ c-'0';
        while (Serial.available())
        {
          c  = Serial.read();
          if (c<'0' || c>'9') return true;
          x = x*10+c-'0';
        }
        return true;
      }
  }
  return false;
  
}
void commandListener()
{
  int ag0, ag1;
  if (Serial.available())
  {
    
    while(Serial.available())
    {
      
      switch (Serial.read())
      {
        case 'i': 
          if (
            readInt(ag0)
            )
          {
            
            //Serial.print("Choose Song ID : ");
            //Serial.println(ag0);
            chooseSID(ag0);
          }
          break;
        case 'v':
        if (
            readInt(ag0) 
            )
          {
            if (!readInt(ag1) ) ag1 = ag0;
            //Serial.print("Change volue : (");
            //Serial.println(ag0);
            //Serial.print(", ");
            //Serial.print(ag1);
            //Serial.println(")"); 
            setupVolume(ag0, ag1);
          }
          break; 
          case 's': MP3.stopTrack();
            break;
          case 'n': nextMusic();
          break;
          case 'p': chooseSID(songID-1);
            break;
          case 'b': if (readInt(ag0))
                MP3.setBassAmplitude(ag0);
                Serial.print("BassAmplitude : ");
                Serial.println( MP3.getBassAmplitude());
                break;
          case 't': if (readInt(ag0))
                MP3.setTrebleAmplitude(ag0);
                 Serial.print("TrebleAmplitude : ");
                Serial.println( MP3.getTrebleAmplitude());
                break;
            
      }
    }
  }
}


/*
 * Sensnor 1 2 3
 * Pin     3 4 5
 * Bit 0  1 to 2
 * Bit 1  2 to 3
 * Bit 3  3 to 1
*/
/*
uint8_t checkSensor()
{
  uint8_t res = 0;
  pinMode(3, OUTPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  
  delay(20);
  
  res |= digitalRead(4);
  res |= digitalRead(5)<<2;
  
  pinMode(4, OUTPUT);
  pinMode(3, INPUT);
  pinMode(5, INPUT);
  digitalWrite(4, HIGH);
  digitalWrite(3, LOW);
  digitalWrite(5, LOW);
  delay(20);
  res |= digitalRead(5)<<1;
  
  
  return res;
  
  
}
*/


bool near(int i, int j)
{
  return i>j?(i-j<15):j-i<15;
}


void onClick(int idBT)
{
  Serial.print("Click");
  Serial.println(idBT);
  switch (idBT)
  {
    case 0: chooseSID(songID-1);
    break ; 
    case 1: nextMusic(); 
    break;
    case 2: setupVolume(volume1 +5, volume2 +5); 
    
    break;
    case 3: setupVolume(volume1 -5, volume2 -5);
    break;
  }
}

int _keyPressed = -1;
void onPress(int idBT)
{
  _keyPressed = idBT;
}

void onRelease()
{
  if (_keyPressed>=0)
  {
    onClick(_keyPressed);
    _keyPressed = -1;
  }
}
int volta[6] = {957,  892, 827, 188, 125, 60};
int getButtonID()
{
  int t =  analogRead(A0);
  for (int i = 0; i< 6; ++i)
  if (near(t, volta[i])) return i;
  return -1; 
}


int lastKey[3] = {-1, -1, -1};
int crKey;
void keyListen()
{
  crKey = getButtonID();
  if ( lastKey[0] == lastKey[1] && lastKey[0] == lastKey[2])
  if (lastKey[2] == crKey)
    onPress(crKey);
    else
    onRelease();
  lastKey[0] = lastKey[1];
  lastKey[1] = lastKey[2];
  lastKey[2] = crKey;
}

void tick50()
{
  keyListen();
}


void sleepForIO()
{
  for ( int i = 0; i< 10; i++)
  {
    tick50();
    delay(50);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(MP3.isPlaying())
  {
    sleepForIO();
  }
  else
  {
    nextMusic();
  }
  //Serial.println("Listening");
  commandListener();
  

}
