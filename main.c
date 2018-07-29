#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char *inputfile;
extern void scan();

int main(int argc, char const *argv[])
{
  if (argc < 2){
    printf("usage: ./main (-f file|-s str)\n");
    return 0;
  }

  if (strcmp(argv[1],"-f")==0){
    FILE *fp = fopen(argv[2], "r");
    if (fp == NULL){
      printf("fopen fail for reason: %s,check file:%s\n", strerror(errno), argv[1]);
      return -1;
    }
    
    fseek(fp, 0L, SEEK_END);
    long filelen = ftell(fp);

    inputfile = (char *)malloc(filelen + 1);
    if (inputfile == NULL){
      printf("malloc fail for reason: %s\n",strerror(errno));
      return -1;
    }

    inputfile[filelen] = '\0';
    fseek(fp, 0L, SEEK_SET);
    fread(inputfile, sizeof(char), filelen, fp);
    
    fclose(fp);
  }else if(strcmp(argv[1],"-s")==0){
    size_t l=strlen(argv[2]);
    inputfile=(char *)malloc(l+1);
    if (inputfile == NULL){
      printf("malloc fail for reason: %s\n",strerror(errno));
      return -1;
    }

    memcpy(inputfile,argv[2],l+1);
  }else{
    printf("usage: ./main (-f file|-s str)!\n");
    return 0;
  }

  //printf("%s\n", inputfile);

  scan();
  
  free(inputfile);
  double x=01.2;
  return 0;
}
