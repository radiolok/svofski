#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Mod.h"

bool showInfo=false;

int main(int argc, char *argv[]){

  if(argc<2){
    printf("Mod2Fras by DannyBoy (dan@netbabyworld.com) 2002\n\n");
    printf("  Usage: Mod2Fras.exe file1.mod <file2.mod ...>\n");
    return 0;
  }

  unsigned short counter;
  for(counter=1;counter<argc;counter++){
    if(stricmp(argv[counter],"-info")==0){
      showInfo=true;
    }
  }

  for(counter=1;counter<argc;counter++){

    bool foundFile=false;
    if(stricmp(argv[counter],"-info")!=0){

      char tempName[100];
      char dataName[100];
      strcpy(tempName,argv[counter]);
      if(strrchr(tempName,'.')!=NULL){
        strcpy(strrchr(tempName,'.'),"\0");
      }
      if(strrchr(tempName,'\\')!=NULL){
        strcpy(dataName,strrchr(tempName,'\\')+1);
      }else{
        strcpy(dataName,tempName);
      }

      FILE *file=fopen(argv[counter],"rb");
      if(file==NULL){
        printf("Error opening file: %s\n",argv[counter]);
      }else{
        if(!DoMod(file,dataName)){
          printf("Couldn't parse file: %s\n",argv[counter]);
          printf("  Currenlty only .mod-files can be parsed\n",argv[counter]);
        }

        fclose(file);
      }
    }
  }

  return 0;
}
