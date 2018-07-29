#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define _MAX_LINE 80
enum symboltype{
  TERMINAL=1,
  NON_TERMINAL,
  START
};
// 文法符号
struct symbol{
  int type;
  char str[8];
  struct symbol *next;
};

void LL1(){
  
}

static int compstr(const void *s1,const void *s2){
  const char *t1,*t2;
  t1=(const char *)s1;
  t2=(const char *)s2;
  return strcmp(t1,t2);
}
int main(int argc,const char *argv[]){
  int snum;
  char (* grammars)[_MAX_LINE];
  
  printf("请输入文法数目:");
  scanf("%d",&snum);
  getchar(); //consume the '\n'
  //printf("文法数目为:%d\n",snum);

  puts("文法范例: (T:=T+T)");
  grammars=(char (*) [_MAX_LINE]) malloc(snum*_MAX_LINE);
  int i=0;
  for(;i<snum;i++){
    printf("请输入第%d个文法:\n",i+1);
    char ch;
    
    if(fgets(grammars[i],_MAX_LINE,stdin)==NULL){
      puts("fgets error!");
    }
    char *p;
    if(p=strchr(grammars[i],'\n')){
	  	*p='\0';
  	}
  }
  puts("输入结束!");
  
  qsort(grammars[0],snum,_MAX_LINE,compstr);
  puts("排序后文法：");
  for(i=0;i<snum;i++){
    printf("%s\n",grammars[i]);
  }

  char (*terminals)[8];
  terminals=(char (*)[8])malloc(8*snum);
  char *cur="";
  int terminalnum=0;
  for(i=0;i<snum;i++){
    int j,k;
    j=strchr(grammars[i],':')-grammars[i];
    for(k=0;k<j;k++){
      terminals[terminalnum][k]=grammars[i][k];
    }
    terminals[terminalnum][k]='\0';

    if(strcmp(cur,terminals[terminalnum])!=0){
      cur=terminals[terminalnum];
      puts(cur);
      terminalnum++;
    }
  }
  free(terminals);
  free(grammars);
}