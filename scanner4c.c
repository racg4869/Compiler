/**
 * scan4c.c:
 * Lexical Analysis Program for C
 * 
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>


#define isletter(s) ((s) == '_' || ((s) >= 'a' && (s) <= 'z') || ((s) >= 'A' && (s) <= 'Z'))

#define isop(s) (((s)=='<') || ((s)=='=') || ((s)=='>') || ((s)=='-') || ((s)=='|') || ((s)=='&') ||\
                ((s)=='!') || ((s)=='/') || ((s)=='*') || ((s)=='+') || ((s)=='[') || ((s)==']') || \
                ((s)=='.') || ((s)==':') || ((s)=='?'))

#define issep(s) (((s)==';')|| ((s)=='(')|| ((s)==')')||((s)=='{') || ((s)=='}')||  ((s)==',')|| ((s)=='\\')) 

#define isescape(s) (((s)=='a') || ((s)=='b')|| ((s)=='f')|| ((s)=='n')|| ((s)=='0') ||\
                    ((s)=='v') || ((s)=='r')|| ((s)=='t') || ((s)=='"')|| ((s)=='\\') ||((s)=='\''))

#define isoct(c) (c>='0' && c<='7')

#define ishex(c) (isdigit(c)|| (toupper(c)<='F' && toupper(c)>='A'))

#define _RESERVED_NUM 32
const char *keywords[_RESERVED_NUM] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "int", "long",
    "register", "return", "short", "signed", "sizeof", "static", "struct", "switch",
    "typedef", "union", "unsigned", "void", "volatile", "while"};

enum keywordtype{
  AUTO = 128,
  BREAK,
  CASE,
  CHAR,
  CONST,
  CONTINUE,
  DEFAULT,
  DO,
  DOUBLE,
  ELSE,
  ENUM,
  EXTERN,
  FLOAT,
  FOR,
  GOTO,
  IF,
  INT,
  LONG,
  REGISTER,
  RETURN,
  SHORT,
  SIGNED,
  SIZEOF,
  STATIC,
  STRUCT,
  SWITCH,
  TYPEDEF,
  UNION,
  UNSIGNED,
  VOID,
  VOLATILE,
  WHILE
};

/**
 * return the index of `s` in keywords if successs
 * return -1 if not reserved
 */
int searchkeywords(const char *s){
  int st, ed, a, b;

  st = 0;
  a = strcmp(s, keywords[0]);
  if (a == 0){
    return st;
  }else if (a < 0){
    return -1;
  }

  ed = _RESERVED_NUM - 1;
  b = strcmp(s, keywords[ed]);
  if (b == 0){
    return ed;
  }else if (b > 0){
    return -1;
  }

  int i, c;
  while (st + 1 < ed){
    i = (st + ed) / 2;
    //printf("%d:%s\n", i, keywords[i]);
    c = strcmp(s, keywords[i]);
    if (c == 0){
      return i;
    }else if (c > 0){
      st = i;
    }else{
      ed = i;
    }
  }
  return -1;
}

extern char *inputfile;
const char *curpos,*forward;
int row=0,col=0;

// skip white space
void skipwhitespace(){
  while (isspace(*curpos)){
    if(*curpos=='\n'){
      row++;
      col=1;
    }else{
      col++;
    }
    curpos++;
  }
}

void scannumber(){
  // st==0 mean error
  int st=1;
  char ch;
  forward=curpos;
  while(ch=*forward){
    switch(st){
      case 1:
        if(ch=='0'){
          st=2;
        }else if(isdigit(ch)){
          st=6;
        }else{
          goto error;
        }
        break;
      case 2:
        if(ch<='7' && ch>='0'){
          st=5;
        }else if (toupper(ch)=='X'){
          st=3;
        }else if(ch=='.'){
          st=7;
        }else{
          goto integer_token;
        }
        break;
      case 3:
        st=(ishex(ch))?4:0;
        break;
      case 4:
        if(! ishex(ch)){
          goto integer_token;
        }
        break;
      case 5:
        if(isoct(ch)){
          st=5;
        }else if(ch=='.'){
          st=7;
        }else{
          goto integer_token;
        }
        break;
      case 6:
        if(! isdigit(ch)){
          if(ch=='.'){
            st=7;
          }else{
            goto integer_token;
          }
        } 
        break;
      case 7:
        if(isdigit(ch)){
          st=8;
        }else{
          goto error;
        }
        break;
      case 8:
        if(isdigit(ch)){
          st=8;
        }else if(toupper(ch)=='E'){
          st=9;
        }else{
          goto float_token;
        }
        break;
      case 9:
        if(isdigit(ch)){
          st=11;
        }else if(ch=='+'|| ch=='-'){
          st=10;
        }else{
          goto error;
        }
        break;
      case 10:
        if(isdigit(ch)){
          st=11;
        }else{
          goto error;
        }
        break;
      case 11:
        if(! isdigit(ch)){
          goto float_token;
        }
        break;
      default:
        error:  printf("error in scan number:\n");
        exit(1);
    }
    forward++;
  }
end_1:
  return ;
integer_token:
  if(toupper(*forward)=='U')
    forward++;
  if(toupper(*forward)=='L')
    forward++;
  goto end_1;
float_token:
  if(toupper(*forward)=='F')
    forward++;
}

/* scan a string litteral use DFA
 * while(ch != '"' || *(forward - 1) == '\\') forward++;
 * `keep` forward at where next token scan should start
 */
void scanstring(){
  enum status{
    error=0,start,status1,status2
  };
  int st=status1;
  char ch;

  forward=curpos+1;
  while(ch=*forward++){
    switch(st){
      case status1:
        if(ch =='"'){
          return ;
        }else if(ch=='\\'){
          st=status2;
        }
        break;
      case status2:
        if(isescape(ch)){
          st=status1;
        }else{
          fprintf(stderr,"illegal escape \\%c!\n",ch);
          exit(1) ;
        }
        break;        
    }
  }
}

void scanchar(){
  int st=1;
  char ch;
  forward=curpos+1;
  while(ch=*forward){
    switch(st){
      case 1:
        st=(ch=='\'')?4:(ch=='\\'?2:3);
        break;
      case 2:
        st=isescape(ch)?3:4;
        break;
      case 3:
        if(ch=='\''){
          forward++;
          return ;
        }else{
          st=4;
        }
        break;
      default:
        fprintf(stderr,"illegal charactor!\n");
        exit(1);
    }
    forward++;
  }
}

void scancomment(){
  char ch;  
  int st=1;
  forward=curpos+1;
  while(ch=*forward){
    switch(st){
      case 1:
        st=(ch=='/')?2:(ch=='*'?3:0);
        break;
      case 2:
        st=(ch=='\n')?5:2;
        break;
      case 3:
        st=(ch=='*')?4:3;
        break;
      case 4:
        st=(ch=='/')?5:3;
        break;
      case 5:
        return ;
        break;
      default:
        fprintf(stderr,"illegal comment!\n");
        exit(1);
    }
    forward++;
  }
}

/**
 * 
 * scan opertors
 * 
 */ 
void scanop(){
  //char ops[8]={'<','=','>','-','!','/','*','+'};

  forward=curpos+1;
  if(*forward=='='){
    forward++;
  }
}
void scansep(){

}

void scanid(){
  char ch;
  forward=curpos+1;
  while(ch=*forward){
      if(isletter(ch) || isdigit(ch)){
        forward++;
      }else{
        return;
      }
  }
}

void scan(){
  char curch,idbuf[128];

  curpos = inputfile;
  skipwhitespace();
  
  while ((curch = *curpos)){
        
    if(isletter(curch)){
      // identifiers
      scanid();

      memcpy(idbuf,curpos,forward-curpos);
      idbuf[forward-curpos]='\0';

      int ind=searchkeywords(idbuf);
      if(ind==-1){
        printf("Token:<%s, identifier>\n",idbuf);
      }else{
        printf("Token:<%s, keyword>\n",idbuf);
      }
    }else if(isdigit(curch)){
      //number
      scannumber();

      printf("Token:<");
      for (const char *ptr = curpos; ptr < forward; ptr++){
        putchar(*ptr);
      }
      printf(", Number>\n");
    }else if(curch=='/'){
      // comment
      forward=curpos+1;
      if(*forward=='/' || *forward=='*'){
        scancomment();
      }else{
        goto op;
      }
    }else if(isop(curch)){
      //operator
op:   forward=curpos+1;
      if(curch=='|' || curch=='&'){
        if(*forward==curch){
          forward++;
        }
      }else if(curch=='.' || curch=='[' || curch==']' || curch==':' || curch=='?'){

      }else if(*forward=='='){
        forward++;
      }
      printf("Token:<");
      for (const char *ptr = curpos; ptr < forward; ptr++){
        putchar(*ptr);
      }
      printf(", operator>\n");
    }else if(issep(curch)){
      forward=curpos+1;
      printf("Token:<%c,seprator>\n",curch);
    }else if (curch == '"'){
      // string 
      scanstring();
      
      printf("Token:<");
      for (const char *ptr = curpos + 1; ptr < forward-1; ptr++){
        putchar(*ptr);
      }
      printf(", string>\n");
    }else if(curch=='\''){
      //char
      scanchar();

      printf("Token:<");
      for (const char *ptr = curpos + 1; ptr < forward-1; ptr++){
        putchar(*ptr);
      }      
      printf(",chracter>\n");
    }else if(curch=='#'){
      //macro
      forward=curpos+1;
      puts("# happened!\n");
    }else{
      printf("else happened at row:%d col:%d!\n",row,col);
      break;
    }

    // scan to next lexeme
    col+=forward-curpos;
    curpos = forward;
    skipwhitespace();
  }
}

int main_1(int argc, char const *argv[]){
  if (argc < 2){
    printf("please give input file!\n");
    return 0;
  }

  FILE *fp = fopen(argv[1], "r");
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

  //inputfile[filelen] = '\0';
  fseek(fp, 0L, SEEK_SET);
  fread(inputfile, sizeof(char), filelen, fp);
  fclose(fp);

  printf("%s\n", inputfile);

  scan();
  
  free(inputfile);

  return 0;
}
