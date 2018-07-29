import collections
import itertools
import functools
import operator

class Grammar():
  # 空
  EPS=''
  EOF='$'
  def __init__(self,lines):
    Production=collections.namedtuple("Production",("nontrm","form"))
    productions=[]
    for line in lines:
      productions.append(Production(*(s.strip() for s in line.split(":"))))
    self.symbols=set((c for c in itertools.chain(*(p.form for p in productions))))
    self.nontrms=set((p.nontrm for p in productions))
    self.terminals=self.symbols-self.nontrms
    self.terminals.add(Grammar.EOF)
    
    self.voidleftrecrusion(productions)
    self.extractleftgcd(productions)
    self.productions=productions
    
    # init analysis table
    self.firstandfoollow()

  def voidleftrecrusion(self,productions):
    """
    消除左递归
    """
    pass

  def extractleftgcd(self,productions):
    """
    提取左公共字符串
    """
    pass

  def firstandfoollow(self):
    nullable=dict({s:False for s in self.symbols})
    
    loopflag=True
    while(loopflag):
      loopflag=False
      for p in self.productions:
        nt=p.nontrm
        # ! 我有这么一个疑惑 
        # 如果 S -> BS  B-> b| \varepsilon 
        # 那么 S 可为空吗？ 
        # 还是说这不是一个正确的文法定义 因为S不存在初值 鸡生蛋，蛋生鸡的问题
        flag=functools.reduce(operator.and_,(nullable[s] for s in p.form),True)
        if flag and not nullable[nt]: # 必须flag =True 并且nullable[nt]=False是第一次修改
          nullable[nt]=flag
          loopflag=True
    print(nullable)

    first=collections.defaultdict(set)
    for s in self.terminals:
      first[s].add(s)

    loopflag=True
    while(loopflag):
      loopflag=False
      for p in self.productions:
        nt=p.nontrm
        for s in p.form:
          oldlen=len(first[nt])
          first[nt].update(first[s])
          if len(first[nt])>oldlen:
            loopflag=True
          if not nullable[s]:
            break
    print(first)

    follow=collections.defaultdict(set)
    follow[self.productions[0].nontrm].add('$')
    loopflag=True
    while(loopflag):
      loopflag=False
      for p in self.productions:
        nt,form=p.nontrm,p.form
        l=len(form)
        for i in range(l):
          s=p.form[i]
          if s in self.nontrms:
            oldlen=len(follow[s])
            j=i+1
            while(j<l):
              follow[s].update(first[form[j]])
              if not nullable[form[j]]:
                break
              j=j+1
            if j==l:
              follow[s].update(follow[nt])
            if oldlen<len(follow[s]):
              loopflag=True
    print(follow)

    analysistable=dict({nt:collections.defaultdict(set) for nt in self.nontrms})
    for p in  self.productions:
      nt,form=p.nontrm,p.form
      frt,i=set(),0
      dnt=analysistable[nt]
      for s in form:
        frt.update(first[s])
        if not nullable[s]:
          break
        i=i+1
      for s in frt:
        dnt[s].add(p)
      if i==len(form):
        for s in follow[nt]:
          dnt[s].add(p)
    print(' ',end='\t')
    for s in self.terminals:
      print('{:^8}'.format(s),end='\t')
    print()
    for nt in self.nontrms:
      print(nt,end='\t')
      dnt=analysistable[nt]
      for s in self.terminals:
        if s in dnt:
          print('{:^8}'.format('{'+';'.join((v.form  for v in dnt[s]))+'}'),end='\t')
        else:
          print('{:^8}'.format('ERROR'),end='\t')
      print()
    
    self.nullable=nullable
    self.first=first
    self.follow=follow
    self.analysistable=analysistable
  
  def parse(self,):
    pass

  def __str__(self):
    return "<Grammar [productions:%s] [nontrms:%s] [terminals:%s]>"%(str(self.productions),str(self.nontrms),str(self.terminals))

if __name__=="__main__":

  n=int(input("输入文法数目："))
  print("分行输入文法，其中文法格式如: A:aBc,A: (注意空串要留空白)")
  
  lines=[]
  for i in range(1,n+1):
    lines.append(input("输入文法%d:  "%(i)))

  g=Grammar(lines)
  print('')
  print(g)
