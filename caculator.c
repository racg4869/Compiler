#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

/**
 * EBNF语法
 * 
 * factor -> ( expr )
 * factor -> number
 * term   -> factor
 * term   -> factor muldiv term
 * expr   -> term
 * expr   -> term addop expr
 * muldiv -> * | /
 * addop -> + | -
 * 
 */

#define isop(c)     ((c)=='+' || (c)=='-' || (c)=='*' || (c)=='/' || (c)=='(' || (c)==')')
#define isaddop(c)  ((c)=='+' || (c)=='-')
#define ismulop(c)  ((c)=='*' || (c)=='/')
#define isbracket(c) ((c)=='(' || (c)==')')

#define isoct(c) ((c)>='0' && (c)<='7')
#define ishex(c) (isdigit(c)|| (toupper(c)<='F' && toupper(c)>='A'))

#define TAB_COLS 8
char *inputfile;
const char *curpos;
int row,col;
const char *rowstart;

enum TokenType{
  integer=0,operator,tokeneof
};
struct Token{
  int tokentype;
  const char *st;
  const char *ed;
} token;

typedef struct Node{
  int nodeType;
  union{
    int op;
    int dt;
  }value;
  int result;
  struct Node *lc,*rc;
}syntaxNode;

syntaxNode* expr();
syntaxNode* term();
syntaxNode* factor();

syntaxNode *makeOpNode(int op){
  syntaxNode *node=(syntaxNode *)malloc(sizeof(syntaxNode));
  node->nodeType=operator;
  node->value.op=op;
  node->result=0;
  node->lc=NULL;
  node->rc=NULL;
  return node;
}
syntaxNode *makeDtNode(int dt){
  syntaxNode *node=(syntaxNode *)malloc(sizeof(syntaxNode));
  node->nodeType=integer;
  node->value.dt=dt;
  node->result=dt;
  node->lc=NULL;
  node->rc=NULL;
  return node;
}
void *Malloc(size_t sz){
  void *ptr= malloc(sz);
  if (ptr == NULL){
    printf("malloc fail for reason: %s\n",strerror(errno));
    exit(1);
  }
  return ptr;
}
void error(const char *msg){
  fprintf(stderr,"%s @ row: %d col : %d\n",msg,row,col);
  char ch;
  while((ch=*rowstart++) && ch!='\n'){
    putchar(ch);
  }
  puts("");
  for(int i=1;i<col;i++){
    putchar(' ');
  }
  puts("↑");
  /*
  while(ch=*(rowstart++) && ch!='\n'){
    putchar(ch);
  }
  */
  exit(1);
}
// skip white space
void skipwhitespace(){
  while (isspace(*curpos)){
    if(*curpos=='\n'){
      row++;
      rowstart=curpos+1;
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
  const char *forward;

  forward=curpos;
  if(! *curpos){
    goto end_1;
  }
  while(ch=*forward){
    switch(st){
      case 1:
        st=ch=='0'?2:(isdigit(ch)?6:0);
        break;
      case 2:
        if(isoct(ch)){
          st=5;
        }else if (toupper(ch)=='X'){
          st=3;
        }else if(ch=='.'){
          st=7;
        }else{
          goto UL_suffix;
        }
        break;
      case 3:
        st=ishex(ch)?4:0;
        break;
      case 4:
        if(! ishex(ch)){
          goto UL_suffix;
        }
        break;
      case 5:
        if(! isoct(ch)){
          if(ch=='.'){
            st=7;
          }else{
            goto UL_suffix;
          }
        }
        break;
      case 6:
        if(! isdigit(ch)){
          if(ch=='.'){
            st=7;
          }else{
            goto UL_suffix;
          }
        } 
        break;
      case 7:
        st=isdigit(ch)?8:0;
        break;
      case 8:
        if(! isdigit(ch)){
          if(toupper(ch)=='E'){
            st=9;
          }else{
            goto F_suffix;
          }
        }
        break;
      case 9:
        st=(ch=='+'|| ch=='-')?11:(isdigit(ch)?11:0);
        break;
      case 10:
        st=isdigit(ch)?11:0;
        break;
      case 11:
        if(! isdigit(ch)){
          goto F_suffix;
        }
        break;
      default:
        //never happed
        ;
    }
    if(st==0){
      col+=forward-curpos;
      error("scan number error:\n");
      exit(1);
    }
    forward++;
  }

  UL_suffix:
    if(toupper(*forward)=='U')
      forward++;
    if(toupper(*forward)=='L')
      forward++;
    goto end_1;
  F_suffix:
    if(toupper(*forward)=='F')
      forward++;
  end_1:
    curpos=forward;
    return ;

}
void scanop(){

}
/**
 * 
 * 每次执行后
 * token中包含此次读取的token的数据
 * curpos指向下个token的开头
 * 
 */
void nextToken(){
  char ch;
  
  skipwhitespace();
  token.st=curpos;
  ch=*curpos;
  if(! ch){
    token.tokentype=tokeneof;
  }else if(isdigit(*curpos)){
    scannumber();
    token.tokentype=integer;
  }else if(isop(*curpos)){
    token.tokentype=operator;
    curpos++;
  }else{
    error("error token!");
  }
  token.ed=curpos;
  col+=token.ed-token.st;
  
  /*
  printf("get token:");
  for(const char *ptr=token.st;ptr<token.ed;ptr++){
    putchar(*ptr);
  }
  puts(";");
  */
}

void match(const char *str){
    int l=strlen(str);
    if(l!=(token.ed-token.st)){
      error(str);
    }
    const char *ptr;
    for (ptr=token.st;ptr<token.ed;ptr++){
      //printf("%c %c\n",*ptr,str[ptr-token.st]);
      if(str[ptr-token.st]!=*ptr){
        error(str);
      }
    }
    nextToken();
}

syntaxNode* factor(){
  syntaxNode *tmp; 
  if(token.tokentype==integer){
    int f;
    char buf[32],*bufptr;
    const char *ptr;
    bufptr=buf;
    for(ptr=token.st;ptr<token.ed;ptr++){
      *(bufptr++)=*ptr;
    }
    *bufptr='\0';
    sscanf(buf,"%d",&f);    
    //printf("factor %s get %d\n",buf,f);

    tmp=makeDtNode(f);
    nextToken();
  }else{
    match("(");
    tmp=expr();
    match(")");
  }
  return tmp;
}
syntaxNode* term(){
  syntaxNode *tmp=factor();
  
  while(token.tokentype==operator && ismulop(*token.st)){
    syntaxNode *newtmp=makeOpNode(*token.st);
    newtmp->lc=tmp;
    if(*token.st=='*'){
      match("*");
      newtmp->rc=factor();
      newtmp->result=newtmp->lc->result*newtmp->rc->result;
    }else{
      match("/");
      newtmp->rc=factor();
      newtmp->result=newtmp->lc->result/newtmp->rc->result;
    }
    tmp=newtmp;
  }
  return tmp;
}
syntaxNode* expr(){
  
  syntaxNode* tmp=term();
  
  while(token.tokentype==operator && isaddop(*token.st)){
    syntaxNode *newtmp=makeOpNode(*token.st);
    newtmp->lc=tmp;
    
    if(*token.st=='+'){
      match("+");
      newtmp->rc=term();
      newtmp->result=newtmp->lc->result+newtmp->rc->result;
    }else if(*token.st=='-'){
      match("-");
      newtmp->rc=term();
      newtmp->result=newtmp->lc->result-newtmp->rc->result;
    }
    tmp=newtmp;
  }
  return tmp;
}
void printTree(syntaxNode *root,int indent){
  int i;
  for(i=0;i<indent;i++){
    putchar('\t');
  }
  if(root->nodeType==integer){
    printf("|-<%d>\n",root->value.dt);
  }else{
    printf("|-<%c>\n",root->value.op);
  }

  if(root->lc){
    printTree(root->lc,indent+1);
  }
  if(root->rc){
    printTree(root->rc,indent+1);
  }
}

// 将兄弟节点连接起来更适合阅读
int linuxPrintTree(syntaxNode *root,int indent){
  int i;
  for(i=0;i<indent;i++){
    putchar('\t');
  }
  if(root->nodeType==integer){
    printf("|-<%d>\n",root->value.dt);
    return 1;
  }else{
    printf("|-<%c>\n",root->value.op);
  }

  int rows=0;
  if(root->lc){
    rows+=linuxPrintTree(root->lc,indent+1);
  }
  if(root->rc){
    if(rows>1){
      printf("%c[%dA%c[%dC",0x1b,rows-1,0x1b,(indent+1)*TAB_COLS);
      for(i=1;i<rows;i++){
        printf("%c%c[%dB%c[%dD",'|',0x1b,1,0x1b,1);
      }
      printf("%c[%dD",0x1b,(indent+1)*TAB_COLS);
    }

    rows+=linuxPrintTree(root->rc,indent+1);
  }
  return rows+1;
}

int main(int argc,const char* argv[]){
  if (argc < 2){
    printf("usage: %s (-f file|-s str)\n",argv[0]);
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

    inputfile = (char *)Malloc(filelen + 1);

    inputfile[filelen] = '\0';
    fseek(fp, 0L, SEEK_SET);
    fread(inputfile, sizeof(char), filelen, fp);
    fclose(fp);
  }else if(strcmp(argv[1],"-s")==0){
    size_t l=strlen(argv[2]);
    inputfile=(char *)Malloc(l+1);

    memcpy(inputfile,argv[2],l+1);
  }else{
    printf("usage: %s (-f file|-s str)!\n",argv[0]);
    return 0;
  }

  curpos = inputfile;
  rowstart = inputfile;
  row=1;
  col=1;
  nextToken();
  
  if(token.tokentype!=tokeneof){
    syntaxNode *tree=expr();
    printf("result:%d\n",tree->result);
    linuxPrintTree(tree,0);
  }

  free(inputfile);
}