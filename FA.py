import collections
import operator
import itertools

def re2NFA(re_str):
  """
  Thompson 算法
  """

class NFA:
  EPS=''
  Rejectst=''
  EmptySet=set()
  EmptyDict=dict()

  def __init__(self,trans,start,accepts,statsdesc=None):
    self.trans=dict(trans)   # dict 
    self.start=start          # object
    self.accepts=accepts      # set
    self.statsdesc=statsdesc  # dict 
    
    self.allStatus=set(trans.keys())
    for d in trans.values():
      self.allStatus.update(set(itertools.chain(*(v for v in d.values()))))
    self.waitStatus=self.allStatus-accepts
    self.symbols=set(itertools.chain(*(d.keys() for d in trans.values())))

  def epsClosure(self,stSet):
    """
    `trans` :   transition table ,以defaultdict(dict)存储着所有状态的转换
    `stSet` :   要计算闭包的状态集合
    """
    trans=self.trans
    
    closure=set()
    toCal=set(stSet)
    while toCal:
      st=toCal.pop()
      closure.add(st)
      toCal.update(trans.get(st,{}).get(NFA.EPS,NFA.EmptySet)-closure)

    return closure

  def toDFA(self):
    """
    利用子集构造法将NFA转换为DFA
    """
    trans,start,accepts=self.trans,self.start,self.accepts

    dfastats=[self.epsClosure([start,]),]
    
    dfatrans=collections.defaultdict(dict)
    curidx=0
    while(curidx<len(dfastats)):
      curdfastat=dfastats[curidx]

      for a in itertools.chain(*(trans.get(nfast,NFA.EmptyDict).keys() for nfast in curdfastat)):
        if not (a==NFA.EPS):
          newdfast=set(itertools.chain(*(trans.get(nfast,NFA.EmptyDict).get(a,NFA.EmptySet) for nfast in curdfastat)))
          newdfast=self.epsClosure(newdfast)

          i=-1
          for idx,olddfast in enumerate(dfastats):
            if olddfast==newdfast:
              i=idx
              break
          if i==-1:
            i=len(dfastats)
            dfastats.append(newdfast)
          
          dfatrans[curidx][a]=i
      curidx=curidx+1

    dfaaccepts=set(i for i,v in enumerate(dfastats) if v & accepts)
    dfastatsdesc=dict({i:str(v) for i,v in enumerate(dfastats)})
    return DFA(dfatrans,0,dfaaccepts,dfastatsdesc)

  def __str__(self):
    return "<%s transition: %s; start:%r; accept: %r; description: %r; >"%(self.__class__.__name__,self.trans,self.start,self.accepts,self.statsdesc)

  def graphviz(self):
    return "digraph H{\n\trankdir=LR;size=\"8,5\"\n\tst[shape=point];"+\
    "".join("\n\tst%s[label=\"%s:%s\"];"%(st,st,self.statsdesc.get(st,'')) for st in self.waitStatus)+\
    "".join("\n\tst%s[shape=doublecircle,label=\"%s:%s\"];"%(st,st,self.statsdesc.get(st,'')) for st in self.accepts)+\
    "\n\tst->st%s;"%(self.start)+\
    "".join("\n\tst%s->st%s[label=\"%s\"];"%(k,t,s if NFA.EPS!=s else 'ε') for k,v in self.trans.items() for s,ts in v.items() for t in ts)+"\n}"



class DFA(NFA):
  def __init__(self,trans,start,accepts,statsdesc=None):
    self.trans=dict(trans)          # dict 
    self.start=start          # object
    self.accepts=accepts      # set
    self.statsdesc=statsdesc  # dict 
    
    # 所有状态（不包含错误状态）
    self.allStatus=set(trans.keys())
    self.allStatus.update(set(itertools.chain(*(d.values() for d in self.trans.values()))))
    # 所有非接受的输入状态
    self.waitStatus=self.allStatus-accepts
    # DFA中的所有的输入字符
    self.symbols=set(itertools.chain(*(d.keys() for d in trans.values())))
    
  def minify(self):
    """
    hopcroft算法
    有限状态机对于一次输入存在三种状态： accept,wait,reject
    """
    
    Rejectst=NFA.Rejectst # DFA中的错误状态 所有未定义的转换都默认为Rejectst
    trans,start,accepts=self.trans,self.start,self.accepts
    allStatus,waitStatus,symbols=self.allStatus,self.waitStatus,self.symbols

    # group 是k阶等价的代表元素的列表
    # 也就是说对任意状态s，在group 中一定存在且仅存在一个元素 与s k阶等价
    # 初始时有分别对应着accept 和wait的2个代表性元素
    a,*_=accepts
    w,*_=waitStatus
    group=[w,a]

    # st2gidx是 状态s对应到与group中唯一与s k阶等价的元素a的下标
    # 所以如果两个状态s1,s2 k阶等价 那么 st2gidx[s1]==st2gidx[s2]
    # 初始时 -1,0,1分别对应着reject,wait和accept
    st2gidx=dict({s:(1 if s in accepts else 0) for s in trans.keys()})
    st2gidx[Rejectst]=-1
    
    tmpst2gidx=st2gidx.copy()

    loopflag=True
    while(loopflag):
      """
      循环开始时 st2gidx中存储着 s 到 与group中与s k阶等价的代表元素a的索引
      循环结束时 st2gidx 变为k+1阶等价的索引映射
      """
      loopflag=False
      for st in allStatus:
        newgroup=True
        l=len(group)
        # 状态 st 与 groupst k+1阶等价的 应当k阶等价
        for i in range(st2gidx[st],l):
          groupst=group[i]
          if st2gidx[st]!=st2gidx[groupst]:
            continue
          # 判断是否k+1阶等价
          equal=True
          for a in symbols:
            # 默认返回错误状态Rejectst
            stNext=trans[st].get(a,Rejectst)
            groupstNext=trans[groupst].get(a,Rejectst)
            if(st2gidx[stNext]!=st2gidx[groupstNext]):
              equal=False
              break
          if equal:
            tmpst2gidx[st]=i
            newgroup=False
            break
        # k+1阶等价中新的代表元素
        if newgroup:
          group.append(st)
          tmpst2gidx[st]=l # group[l]=st
          loopflag=True

        # 如果group 发生改变则将st2gidx更新为k+1阶等价的映射并继续循环
        if loopflag:
          for k,v in tmpst2gidx.items():
            st2gidx[k]=v
    
    st2gidx.pop(Rejectst)
    
    st2gidx_l=list((k,v) for k,v in st2gidx.items())
    st2gidx_l.sort(key=operator.itemgetter(1))
    
    dfastats=set()
    dfaaccepts=set()
    dfastatsdesc=dict()
    dfatrans=collections.defaultdict(dict)
    
    for idx,items in itertools.groupby(st2gidx_l,key=operator.itemgetter(1)):
      dfastats.add(idx)
      dfastatsdesc[idx]=set(tp[0] for tp in items)
      if start in dfastatsdesc[idx]:
        dfastart=idx
      if dfastatsdesc[idx] & accepts:
        dfaaccepts.add(idx)
      for a in symbols:
        v=trans[group[idx]].get(a,Rejectst)
        if not v==Rejectst:
          dfatrans[idx][a]=st2gidx[v]

    return DFA(dfatrans,dfastart,dfaaccepts,dfastatsdesc)

  def graphviz(self):
    return "digraph H{\n\trankdir=LR;size=\"8,5\";\n\tst[shape=point];"+\
    "".join("\n\tst%s[label=\"%s:%s\"];"%(st,st,self.statsdesc.get(st,'')) for st in self.waitStatus)+\
    "".join("\n\tst%s[shape=doublecircle,label=\"%s:%s\"];"%(st,st,self.statsdesc.get(st,'')) for st in self.accepts)+\
    "\n\tst->st%s;"%(self.start)+\
    "".join("\n\tst%s->st%s[label=\"%s\"];"%(k,t,s) for k,v in self.trans.items() for s,t in v.items())+"\n}"

  def test(self,syms):
    st=self.start
    for sym in syms:
      st=self.trans.get(st,NFA.EmptyDict).get(sym,NFA.Rejectst)
      if st==NFA.Rejectst:
        return False
    return st in self.accepts

if __name__=="__main__":
  
  n,m=map(int,input("输入有限状态机路线数目和终结状态数目：").split(','))
  # transition table
  print("分行输入状态转换(文法格式如: s1,a,s2   其中`s1`,`s2`为状态;`a`为输入)")
  trans=collections.defaultdict(dict)
  for s1,a,s2 in (input("输入状态转换%d:  "%(i)).strip().split(",") for i in range(1,n+1)):
    if a not in trans[s1]:
      trans[s1][a]={s2,}
    else:
      trans[s1][a].add(s2)
  # start status
  print("输入开始状态:")
  start=input("开始状态：").strip()
  # accept status
  print("分行输入接受状态：")
  accepts=set(input("输入接受状态%d:  "%(i)).strip() for i in range(1,m+1))
  
  
  # 
  print("")
  
  nfa=NFA(trans,start,accepts,{})
  print(nfa) 
  print(nfa.graphviz())
  
  dfa=nfa.toDFA()
  print(dfa)
  print(dfa.graphviz())

  mdfa=dfa.minify()
  print(mdfa)
  print(mdfa.test('ab'))
  print(mdfa.graphviz())


"""
with open("D:/Code/C/Compiler/FA.txt","r") as f:
  lines=f.readlines()
  n,m=map(int,lines[0].split(','))
  trans=collections.defaultdict(dict)
  for s1,a,s2 in (lines[i].strip().split(",") for i in range(1,n+1)):
  if a not in trans[s1]:
    trans[s1][a]={s2,}
  else:
    trans[s1][a].add(s2)

  start=lines[n+1].strip()
  accepts=set(lines[i+n+1].strip() for i in range(1,m+1))
"""
