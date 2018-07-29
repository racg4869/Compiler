# Compiler

基于龙书和《编译原理及实践》的编译原理实践项目

目前已经实践了的功能有：

1) `FA.py`
 基于Python实践了从NFA到DFA的子集构造法和 DFA简化的 Hopcroft算法
2) `parse.py`
 基于Python实践了从消除了左递归的文法生成First Follow和分析表
3) `scanner4c.c`
 基于DFA实现了基于C的 C语言的的词法分析器
4) `caculator.c`
 基于递归下降分析了简单的加减乘除的计算器的分析树和计算

还有**待实现和加强**的功能:

1) `FA.py`
 因为涉及到正则表达式的语法分析，还未实现正则表达式到NFA的Thompson构造
2) `parse.py`
 还未能自动实现消除左递归和提取左公因子
3) `parse4c.c`
 还未能实现基于递归下降分析或LL(1)或者LR(1)的C语言的语法分析 
4) `scanner4dot.c`
 dot 语言的词法分析器
