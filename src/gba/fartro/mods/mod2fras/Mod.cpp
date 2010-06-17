#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Main.h"

//30 bytes
typedef struct{
  char            name[22];
  unsigned short  length;
  unsigned char   fineTune;
  unsigned char   linearVolume;
  unsigned short  repeatOffset;
  unsigned short  repeatLength;
} modSampleInfo;

typedef struct{
  char            songName[20];
  modSampleInfo   samples[31];
  unsigned char   nrOfPatterns;
  unsigned char   songEndJumpPos;
  unsigned char   patternTable[128];
  char            formatTag[4];
} modHeaderLong;

typedef struct{
  char            songName[20];
  modSampleInfo   samples[15];
  unsigned char   nrOfPatterns;
  unsigned char   songEndJumpPos;
  unsigned char   patternTable[128];
} modHeaderShort;

typedef struct{
  unsigned char   nrOfSamples;
  
  char            songName[20];
  modSampleInfo   samples[31];
  unsigned char   nrOfPatterns;
  unsigned char   songEndJumpPos;
  unsigned char   patternTable[128];
  char            formatTag[4];
  unsigned char   nrOfPatternsInFile;
  unsigned char   nrOfChannels;
} modHeaderGeneral;

const unsigned short modFineTuneList[60]={
  1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960 , 906,
    856 , 808 , 762 , 720 , 678 , 640 , 604 , 570 , 538 , 508 , 480 , 453,
    428 , 404 , 381 , 360 , 339 , 320 , 302 , 285 , 269 , 254 , 240 , 226,
    214 , 202 , 190 , 180 , 170 , 160 , 151 , 143 , 135 , 127 , 120 , 113,
    107 , 101 , 95  , 90  , 85  , 80  , 75  , 71  , 67  , 63  , 60  , 56 
};

bool CheckModHeaderShort(modHeaderShort hdr,modHeaderGeneral *hdrGen){

  hdrGen->nrOfSamples=15;

  unsigned char counter;
  for(counter=0;counter<20;counter++){
    if(hdr.songName[counter]!=0){
      if((hdr.songName[counter]<' ')||
        (hdr.songName[counter]>'~')){
        return false;
      }
    }
  }
  strcpy(hdrGen->songName,hdr.songName);
  hdrGen->nrOfPatterns=hdr.nrOfPatterns;
  if(hdrGen->nrOfPatterns>128){
    return false;
  }
  hdrGen->songEndJumpPos=hdr.songEndJumpPos;
  if(hdrGen->songEndJumpPos>128){
    return false;
  }
  memcpy(hdrGen->patternTable,hdr.patternTable,128);
  strncpy(hdrGen->formatTag,"    ",4);
  
  for(counter=0;counter<15;counter++){
    unsigned char counter2;
    for(counter2=0;counter2<20;counter2++){
      if(hdr.songName[counter2]!=0){
        if((hdr.songName[counter2]<' ')||
          (hdr.songName[counter2]>'~')){
          return false;
        }
      }
    }
    strcpy(hdrGen->samples[counter].name,hdr.samples[counter].name);
    hdrGen->samples[counter].length=((hdr.samples[counter].length&0x00ff)<<8)+(hdr.samples[counter].length>>8);
    hdrGen->samples[counter].length*=2;
    hdrGen->samples[counter].fineTune=hdr.samples[counter].fineTune;
    if((hdrGen->samples[counter].fineTune&0xF0)!=0){
      return false;
    }
    hdrGen->samples[counter].linearVolume=hdr.samples[counter].linearVolume;
    if(hdrGen->samples[counter].linearVolume>64){
      return false;
    }
    hdrGen->samples[counter].repeatOffset=((hdr.samples[counter].repeatOffset&0x00ff)<<8)+(hdr.samples[counter].repeatOffset>>8);
    hdrGen->samples[counter].repeatOffset*=2;
    hdrGen->samples[counter].repeatLength=((hdr.samples[counter].repeatLength&0x00ff)<<8)+(hdr.samples[counter].repeatLength>>8);
    hdrGen->samples[counter].repeatLength*=2;
    if(hdrGen->samples[counter].repeatLength<=2){
      hdrGen->samples[counter].repeatLength=hdrGen->samples[counter].length;
      hdrGen->samples[counter].repeatOffset=hdrGen->samples[counter].repeatLength;
    }else{
      hdrGen->samples[counter].repeatLength+=hdrGen->samples[counter].repeatOffset;
    }
  }

  hdrGen->nrOfPatternsInFile=0;
  for(counter=0;counter<128;counter++){
    if(hdr.patternTable[counter]>hdrGen->nrOfPatternsInFile){
      hdrGen->nrOfPatternsInFile=hdr.patternTable[counter];
    }
  }
  hdrGen->nrOfPatternsInFile++;
  hdrGen->nrOfChannels=4;

  return true;
}

bool CheckModHeaderLong(modHeaderLong hdr,modHeaderGeneral *hdrGen){

  hdrGen->nrOfSamples=31;
  unsigned char counter;
  for(counter=0;counter<20;counter++){
    if(hdr.songName[counter]!=0){
      if((hdr.songName[counter]<' ')||
        (hdr.songName[counter]>'~')){
        return false;
      }
    }
  }
  strcpy(hdrGen->songName,hdr.songName);
  hdrGen->nrOfPatterns=hdr.nrOfPatterns;
  if(hdrGen->nrOfPatterns>128){
    return false;
  }
  hdrGen->songEndJumpPos=hdr.songEndJumpPos;
  if(hdrGen->songEndJumpPos>128){
    return false;
  }
  memcpy(hdrGen->patternTable,hdr.patternTable,128);
  strncpy(hdrGen->formatTag,hdr.formatTag,4);

  hdrGen->nrOfChannels=0xFF;
  if((strncmp(hdrGen->formatTag,"M.K.",4)==0)||
    (strncmp(hdrGen->formatTag,"FLT4",4)==0)||
    (strncmp(hdrGen->formatTag,"M!K!",4)==0)||
    (strncmp(hdrGen->formatTag,"4CHN",4)==0)){
    hdrGen->nrOfChannels=4;
  }
  if((strncmp(hdrGen->formatTag,"8CHN",4)==0)||
    (strncmp(hdrGen->formatTag,"CD81",4)==0)||
    (strncmp(hdrGen->formatTag,"OCTA",4)==0)){
    hdrGen->nrOfChannels=8;
  }
  if((strncmp(hdrGen->formatTag,"2CHN",4)==0)||
    (strncmp(hdrGen->formatTag,"TDZ2",4)==0)){
    hdrGen->nrOfChannels=2;
  }
  if(strncmp(hdrGen->formatTag,"16CH",4)==0){
    hdrGen->nrOfChannels=16;
  }
  if(strncmp(hdrGen->formatTag,"32CH",4)==0){
    hdrGen->nrOfChannels=32;
  }

  if(hdrGen->nrOfChannels==0xFF){
    return false;
  }

  for(counter=0;counter<31;counter++){
    unsigned char counter2;
    for(counter2=0;counter2<20;counter2++){
      if(hdr.songName[counter2]!=0){
        if((hdr.songName[counter2]<' ')||
          (hdr.songName[counter2]>'~')){
          return false;
        }
      }
    }
    strcpy(hdrGen->samples[counter].name,hdr.samples[counter].name);
    hdrGen->samples[counter].length=((hdr.samples[counter].length&0x00ff)<<8)+(hdr.samples[counter].length>>8);
    hdrGen->samples[counter].length*=2;
    hdrGen->samples[counter].fineTune=hdr.samples[counter].fineTune;
    if((hdrGen->samples[counter].fineTune&0xF0)!=0){
      return false;
    }
    hdrGen->samples[counter].linearVolume=hdr.samples[counter].linearVolume;
    if(hdrGen->samples[counter].linearVolume>64){
      return false;
    }
    hdrGen->samples[counter].repeatOffset=((hdr.samples[counter].repeatOffset&0x00ff)<<8)+(hdr.samples[counter].repeatOffset>>8);
    hdrGen->samples[counter].repeatOffset*=2;
    hdrGen->samples[counter].repeatLength=((hdr.samples[counter].repeatLength&0x00ff)<<8)+(hdr.samples[counter].repeatLength>>8);
    hdrGen->samples[counter].repeatLength*=2;
    if(hdrGen->samples[counter].repeatLength<=2){
      hdrGen->samples[counter].repeatLength=hdrGen->samples[counter].length;
      hdrGen->samples[counter].repeatOffset=hdrGen->samples[counter].repeatLength;
    }else{
      hdrGen->samples[counter].repeatLength+=hdrGen->samples[counter].repeatOffset;
    }
  }

  hdrGen->nrOfPatternsInFile=0;
  for(counter=0;counter<128;counter++){
    if(hdr.patternTable[counter]>hdrGen->nrOfPatternsInFile){
      hdrGen->nrOfPatternsInFile=hdr.patternTable[counter];
    }
  }
  hdrGen->nrOfPatternsInFile++;

  return true;
}


unsigned DoMod(FILE *modFile,char *dataName){

  if(fseek(modFile,470,SEEK_SET)!=0){
    return false;
  }

  unsigned char buffy;
  if(fread(&buffy,1,1,modFile)!=1){
    return false;
  }

  if(fseek(modFile,0,SEEK_SET)!=0){
    printf("Serious IO-error while checking mod-header of file: %s\n",dataName);
    return true;
  }
  
  modHeaderGeneral hdrGen;
  if((buffy==0)||((buffy>=32)&&(buffy<=126))){
    modHeaderLong hdr;
    if(fread(&hdr,sizeof(hdr),1,modFile)!=1){
      return true;
    }
    
    if(CheckModHeaderLong(hdr,&hdrGen)==false){
      return false;
    }
  }else{
    modHeaderShort hdr;
    if(fread(&hdr,sizeof(hdr),1,modFile)!=1){
      printf("Serious IO-error while checking mod-header of file: %s\n",dataName);
      return true;
    }
    
    if(CheckModHeaderShort(hdr,&hdrGen)==false){
      return false;
    }
  }

  if(showInfo){
    printf("\nName: %s,mod\n",dataName);
    printf("Song name: %s\n",hdrGen.songName);
    printf("Format tag: %c%c%c%c\n",hdrGen.formatTag[0],hdrGen.formatTag[1],hdrGen.formatTag[2],hdrGen.formatTag[3]);
    printf("# of channels: %u\n",hdrGen.nrOfChannels);
    printf("# of patterns: %u\n",hdrGen.nrOfPatterns);
    printf("# of patterns in file: %u\n",hdrGen.nrOfPatternsInFile);
    printf("# of samples: %u\n",hdrGen.nrOfSamples);
  }

  char cFileName[100];
  strcpy(cFileName,dataName);
  strcat(cFileName,".c");
  
  char hFileName[100];
  strcpy(hFileName,dataName);
  strcat(hFileName,".h");
  
  FILE *cFile=fopen(cFileName,"wb");
  if(!cFileName){
    printf("Error opening c-file: %s\n",cFileName);
    return true;
  }

  FILE *hFile=fopen(hFileName,"w");
  if(!hFile){
    printf("Error opening h-file: %s\n",hFileName);
    return true;
  }

  char upperCase[100];
  strcpy(upperCase,dataName);
  strupr(upperCase);


  fprintf(hFile,"#ifndef %s_H\n",upperCase);
  fprintf(hFile,"#define %s_H\n\n",upperCase);
  
  fprintf(hFile,"#ifndef FRASMODINFO\n");
  fprintf(hFile,"#define FRASMODINFO\n\n");

  fprintf(hFile,"typedef struct{\n");
  fprintf(hFile,"  const unsigned length;\n");
  fprintf(hFile,"  const unsigned fineTune;\n");
  fprintf(hFile,"  const unsigned linearVolume;\n");
  fprintf(hFile,"  const unsigned repeatOffset;\n");
  fprintf(hFile,"  const char     *sampleData;\n");
  fprintf(hFile,"} sampleInfo;\n\n");
  
  fprintf(hFile,"typedef struct{\n");
  fprintf(hFile,"  const sampleInfo     *sample;\n");
  fprintf(hFile,"  const unsigned short period;\n");
  fprintf(hFile,"  const unsigned short effect;\n");
  fprintf(hFile,"} patternChannelInfo;\n\n");
  
  fprintf(hFile,"typedef struct{\n");
  fprintf(hFile,"  const unsigned   nrOfPatterns;\n");
  fprintf(hFile,"  const unsigned   nrOfChannels;\n");
  fprintf(hFile,"  const patternChannelInfo *patternTable[];\n");
  fprintf(hFile,"} modInfo;\n\n");
  fprintf(hFile,"#endif\n\n");
  
  
  fprintf(hFile,"extern const modInfo %sModInfo;\n\n",dataName);

  fprintf(hFile,"#endif\n");
  fclose(hFile);

  fprintf(cFile,"#define NULL ((void*)0)\n\n");
  
  fprintf(cFile,"#include \"%s\"\n\n",hFileName);
  

  //save pos of the patterns for later ref
  long patternStart=ftell(modFile);

  //move to where the samples are located
  
  
  fseek(modFile,hdrGen.nrOfPatternsInFile*(hdrGen.nrOfChannels*4*64),SEEK_CUR);
  
  //samples
  unsigned long counter;
  for(counter=0;counter<hdrGen.nrOfSamples;counter++){

    if(hdrGen.samples[counter].length<2){
      continue;
    }
    
    fprintf(cFile,"const char %sSampleData%u[%u]={\n  ",dataName,counter+1,hdrGen.samples[counter].repeatLength);
    unsigned long sampleCounter;
    
    char data;
    if(fread(&data,1,1,modFile)!=1){
      printf("Error reading sample data of file: %s\n",dataName);
      return true;
    }
    if(fread(&data,1,1,modFile)!=1){
      printf("Error reading sample data of file: %s,mod\n",dataName);
      return true;
    }

    for(sampleCounter=2;sampleCounter<hdrGen.samples[counter].length;sampleCounter++){

      if(fread(&data,1,1,modFile)!=1){
        printf("Warning: Sample %u was truncated at %u/%u in file: %s\n",counter,sampleCounter,hdrGen.samples[counter].length,dataName);
        break;
      }

      if(sampleCounter<=hdrGen.samples[counter].repeatLength){
        fprintf(cFile,"%4d,",data);
        if(sampleCounter%10==9){
          fprintf(cFile,"\n  ");
        }
      }
    }
    fprintf(cFile,"};\n\n");
    
    
    fprintf(cFile,"const sampleInfo %sSampleInfo%u={\n",dataName,counter+1);
    
    fprintf(cFile,"  //length of sample\n");
    fprintf(cFile,"  %u,\n",hdrGen.samples[counter].repeatLength);
    
    fprintf(cFile,"  //finetune of sample\n");
    fprintf(cFile,"  %u,\n",hdrGen.samples[counter].fineTune);
    
    fprintf(cFile,"  //linear volume of sample\n");
    fprintf(cFile,"  %u,\n",hdrGen.samples[counter].linearVolume);
    
    fprintf(cFile,"  //repeat offset of sample\n");
    fprintf(cFile,"  %u,\n",hdrGen.samples[counter].repeatOffset);
    
    fprintf(cFile,"  //sample data\n");
    fprintf(cFile,"  %sSampleData%u\n};\n\n",dataName,counter+1);
  }

  //move back to the pattern pos
  fseek(modFile,patternStart,SEEK_SET);

  //patterns
  for(counter=0;counter<hdrGen.nrOfPatternsInFile;counter++){
    bool foundPattern=false;
    unsigned patternCounter;
    for(patternCounter=0;patternCounter<hdrGen.nrOfPatterns;patternCounter++){
      if(hdrGen.patternTable[patternCounter]==counter){
        foundPattern=true;
      }
    }

    if(foundPattern){
      fprintf(cFile,"const patternChannelInfo %sPatternData%u[%u]={\n",dataName,counter,hdrGen.nrOfChannels*64);
      
      unsigned short lineCounter;
      for(lineCounter=0;lineCounter<64;lineCounter++){
        
        fprintf(cFile,"  //line %u\n",lineCounter);
        
        unsigned short channelCounter;
        for(channelCounter=0;channelCounter<hdrGen.nrOfChannels;channelCounter++){
          unsigned char sample;
          unsigned short period;
          unsigned short note;
          unsigned short effect;
          
          unsigned char data;
          if(fread(&data,1,1,modFile)!=1){
            printf("Error reading pattern data of file: %s\n",dataName);
            return true;
          }
          sample=data&0xf0;
          period=(data&0x0f)<<8;
          
          if(fread(&data,1,1,modFile)!=1){
            printf("Error reading pattern data of file: %s\n",dataName);
            return true;
          }
          period+=data;
          
          if(fread(&data,1,1,modFile)!=1){
            printf("Error reading pattern data of file: %s\n",dataName);
            return true;
          }
          sample+=(data&0xf0)>>4;
          effect=(data&0x0f)<<8;
          
          if(fread(&data,1,1,modFile)!=1){
            printf("Error reading pattern data of file: %s\n",dataName);
            return true;
          }
          effect+=data;

          switch(effect>>8){
          case 0x00:
            break;
          case 0x01:
            break;
          case 0x02:
            break;
          case 0x04:
            break;
          case 0x06:
            if(((effect&0xF0)!=0)&&((effect&0x0F)!=0)){
              effect=0;
            }
            break;
          case 0x09:
            break;
          case 0x0A:
            if(((effect&0xF0)!=0)&&((effect&0x0F)!=0)){
              effect=0;
            }
            break;
          case 0x0B:
            if((effect&0xFF)>=hdrGen.nrOfPatterns){
              effect=0x0B00+(hdrGen.nrOfPatterns-1);
            }
            break;
          case 0x0C:
            if((effect&0xFF)>64){
              effect=0x0C40;
            }
            break;
          case 0x0D:
            if((effect&0xFF)>=64){
              effect=0x0D00;
            }
            break;
          case 0x0E:
            effect=0;
            break;
          case 0x0F:
            break;
          default:
            //printf("Effect %04x is not yet supported\n",effect);
            break;
          }
          
          if(period!=0){
            //find the correct periodvalue from list
            unsigned char periodCounter;
            unsigned short usedPeriod;
            unsigned short deltaPeriod=0xffff;
            bool foundPeriod=false;
            for(periodCounter=0;periodCounter<60;periodCounter++){
              if(period==modFineTuneList[periodCounter]){
                note=(periodCounter+1)*8;
                periodCounter=60;
                foundPeriod=true;
              }else{
                unsigned short delta=abs(period-modFineTuneList[periodCounter]);
                
                if(delta<deltaPeriod){
                  deltaPeriod=delta;
                  note=(periodCounter+1)*8;
                  usedPeriod=modFineTuneList[periodCounter];
                }
              }
            }
/*            if(foundPeriod==false){
              printf("Warning: Could't find period value. Choosing closest value instead.\n");
              printf("  File: %s\n",dataName);
              printf("  Pattern: %u\n",counter);
              printf("  Line: %u\n",lineCounter);
              printf("  Channel: %u\n",channelCounter);
              printf("  Found: %u\n",period);
              printf("  Used: %u\n",usedPeriod);
            }
*/          }else{
            note=0;
          }
          if(note>=488){
            printf("what the fuck???????\n");
          }
          
          if(sample==0){
            fprintf(cFile,"  {NULL,0x%04x,0x%04x},\n",note,effect);
          }else{
            if(hdrGen.samples[sample-1].length<2){
              fprintf(cFile,"  {NULL,0x%04x,0x%04x},\n",note,effect);
            }else{
              fprintf(cFile,"  {&%sSampleInfo%u,0x%04x,0x%04x},\n",dataName,sample,note,effect);
            }
          }
        }
      }
      fprintf(cFile,"};\n\n");
    }else{
      for(patternCounter=0;patternCounter<(unsigned)(64*hdrGen.nrOfChannels*4);patternCounter++){
        unsigned char data;
        if(fread(&data,1,1,modFile)!=1){
          printf("Error reading pattern data of file: %s\n",dataName);
          return true;
        }
      }
    }
  }


  //mod info
  fprintf(cFile,"const modInfo %sModInfo={\n",dataName);
  
  fprintf(cFile,"  //nrOfPatterns\n");
  fprintf(cFile,"  %u,\n",hdrGen.nrOfPatterns);
  
  fprintf(cFile,"  //nrOfChannels\n");
  fprintf(cFile,"  %u,\n",hdrGen.nrOfChannels);
  
  fprintf(cFile,"  //patternTable[%u]\n  {\n",hdrGen.nrOfPatterns);
  for(counter=0;counter<hdrGen.nrOfPatterns;counter++){
    fprintf(cFile,"    %sPatternData%u,\n",dataName,hdrGen.patternTable[counter]);
  }
  fprintf(cFile,"  },\n");
  fprintf(cFile,"};\n\n");
  
  fclose(modFile);
  fclose(cFile);

  return true;
}

