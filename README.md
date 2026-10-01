# 一个自己写的 Python 解释器（C++ / Qt，字节码虚拟机）

从零实现的 Python 子集解释器 + 一个带界面的小型 IDE。**词法、语法、AST、编译器、字节码虚拟机、对象模型、作用域与闭包、类与继承全部手写**，没有用任何现成的解析器生成器或虚拟机库。

这个仓库是这条学习路线上的**第三代**：

| 代 | 位置 | 形态 |
|---|---|---|
| 第一代 | `Tokenizer` 仓库 | 最早的**原型**：`Parser → AST → Executor` 直接遍历求值（tree-walking），当时对编译原理还不熟 |
| 第二代 | 本仓库的 `begin` 分支 | 搬到 Qt Creator，加界面，仍是 tree-walking |
| **第三代** | **本仓库 `master`** | 搬到 Visual Studio 重写为 **compiler → 字节码 → 虚拟机** 架构；因为调试更可控，也长出了完整的类系统 |

> 界面（`.ui`）是在 Qt Creator 里设计好再搬进这个 VS 工程的，所以仓库里能看到 `mainwindow.ui`、`newprojectwidget.ui`、`SecondaryWindow.ui` 这些 Qt 痕迹 —— 这是移植历史留下的印记。

## 管道

```
源码 (.py)
  → Lex::Lexer       词法分析：Token 流（含 INDENT / DEDENT）
  → Parse::Parser    语法分析：AST（27 种节点）
  → Compile::Compiler 编译：指令序列 QVector<Instruction>
  → vm::PVM          执行：栈式虚拟机的取指-译码-执行循环
```

`Core`（`Core.h/.cpp`）是这条管道的门面，依次驱动四个阶段，并把**分词/语法/编译/执行**四类错误分别捕获、输出到界面。

## 语言已经支持到什么程度

**关键字**（`lexer.h` 的 `KEYWORDS`）：
`if` `else` `elif` `while` `for` `in` `break` `continue` `pass` `def` `return` `class` `super` `import` `global` `print` `input` `True` `False` `and` `or` `not`

**语法与语义**：

| 类别 | 支持的内容 |
|---|---|
| 字面量 | 整数、浮点、字符串（单/双引号）、`True`/`False`、列表 `[...]`、字典 `{k: v, ...}` |
| 表达式 | 算术、比较、逻辑、一元运算、下标 `a[i]`、属性 `a.b`、函数/方法调用 |
| 运算符优先级 | `or` → `and` → `in` → 比较 → `+ -` → `* / %` → 一元 → 基本元素（标准 Python 层次） |
| 语句 | 赋值（变量/下标/属性三种左值）、`print`、`if/elif/else`、`while`、`for ... in ...`、`break`、`continue`、`return`、`pass` |
| 函数 | 位置参数 + 关键字参数 + **默认参数**；调用时支持位置/关键字混用 |
| 类 | 类定义、**多继承**、实例化、属性、实例方法、`self` 绑定、**C3 MRO**、`super()`（0/1/2 参数三种形式）、`__init__`、运算符重载（`__add__` 等 dunder 分派）、属性拦截（`__getattr__` / `__setattr__`） |
| 作用域 | 环境链（`parentEnvir`）、`global` 声明、块级作用域；**闭包环境的结构已经全部接好**（函数对象在定义处捕获环境 + 查找链上预留了闭包这一环），但嵌套函数访问外层局部变量**还没真正跑通** —— 这部分在本地未提交的修改里尝试过、做到一半（帧被改乱了），所以没有出现在仓库的 `master` 上 |
| 模块 | `import`：按项目结构寻找 `.py`、独立跑一遍、把结果环境包装成 `PModel` 对象绑定到当前作用域 |
| 内置 | `print`、`len`（`registerGlobalFunctions` / `registerLen`）、`PList.append`（对象内置方法） |

## 架构要点

### 1. 编译器是两遍 + 递归下沉

`Compiler::compileAST()` 的顺序是：

```cpp
compileFunctions(ast);   // 预扫所有顶层 def，先生成函数对象
compileClasses(ast);     // 预扫所有顶层 class
compileBlock(ast);       // 再编主流程
cache.push_back(HALT);
```

函数/类在"定义处"就把对象造好并绑定到名字上，所以主流程里遇到 `def`/`class` 节点直接跳过（`compileStatement` 里那两个 `break`）。

**每个函数体被编译成独立的指令序列**，再 `Instruction::toByteArray()` 序列化成一个 `QByteArray`，作为 `CREATE_FUNCTION` 的操作数存进函数对象 —— 这就是这个解释器的"代码对象"形态（而且 `Instruction` 重载了 `QDataStream` 的 `<<`/`>>`，字节码可以落盘）。

函数参数在编译期被拆成两部分：位置参数编译成一个列表，默认参数编译成一个字典（`{名: 默认值}`），运行期由 `PFunction::__call__` 完成绑定。

### 2. 虚拟机是栈式，三套栈

| 栈 | 作用 |
|---|---|
| `valueStack` | 求值栈（`QStack<PObject::pointer>`） |
| `callFrameStack` | 调用帧：返回地址、返回环境、**当前代码、`selfInstance`、`funcBeloneClass`、caller** |
| `blockFrameStack` | 块帧：`breakPC` / `continuePC`。`break`/`continue` 从栈顶**倒着找**第一个循环帧再跳；循环体结束由 `LOOP_*_END` 弹帧 |

`CallFrame` 里带着 `selfInstance` 与 `funcBeloneClass`，所以 `self` 与 `super()` 的实现是"从当前调用帧取"：

```cpp
pointer PVM::getCurrSelfInstance()    { return callFrameStack.top().selfInstance; }
pointer PVM::getCurrFuncBelongClass() { return callFrameStack.top().funcBeloneClass; }
```

每次调用 push 一个帧，`RETURN` 弹出并把 `currCodes` 换回上一帧的字节码 —— 函数调用、环境切换、代码切换三件事是一体的。

### 3. 对象模型对齐了 Python 的数据模型

`pobject.h` 里的 `PObject` 定义了一整套虚函数协议，各类型各自覆写：

- 算术：`__add__` `__sub__` `__mul__` `__truediv__` `__mod__` `__pow__`
- 比较：`__eq__` `__ne__` `__lt__` `__le__` `__gt__` `__ge__`
- 逻辑/迭代：`__and__` `__or__` `__not__`、`__iter__` `__next__`
- 容器：`__getitem__` `__setitem__`
- 类机制：`__call__` `__instance__` `__getattribute__` `__setattribute__` `__mro__`

类型用 `enum class Type` + `PyType` 包装（`TypeToString` / `PObject::getType()`），还有一批 `default*Error()` 统一生成"该类型不支持此操作"的报错。

### 4. 继承用的是真正的 C3 线性化

`PClass::mro()` 完整实现了 C3：递归合并各父类的 MRO 与直接父类列表，每轮取一个候选 head，用 `neverInTail()` 检查它**没有出现在任何列表的尾部**（跳过索引 0，这是 C3 的关键），通过才收进结果。合并失败时报 `C3 MRO 错误：无法合并继承`。

结果缓存在 `mrolist` 里，`PClass::__getattribute__` 沿 MRO 逐个类用 `onlygeattributehere()` 查（先函数表、再静态成员表）——**属性查找是遵守 MRO 的**。

`PSuper` 的语义与 CPython 一致：拿到实例的 MRO，找到当前类的位置，返回它的下一个：

```cpp
QVector<pointer> mro = instance->__mro__();
auto index = mro.indexOf(currClass);
if (mro[index + 1] != PClass::object) return mro[index + 1];
```

### 5. 作用域与闭包

`Environment` 是一条父链（`parentEnvir`），变量用 `QMap<QString, pointer>` 存。查找顺序是：全局声明 → 自己 → **闭包环境** → 父环境链。函数对象在创建时捕获 `closureEnv`（`create_function_execute` 里传的 `currEnvir()`），所以嵌套函数能访问外层变量。

`global` 用一张 `QSet<QString>* globalVars` 贯通所有环境：声明后，赋值与读取都直接走到链根。

### 6. 工程与导入

`Project` 负责识别"项目根 / src 目录"，`import` 会 `findFile(...)` 找到目标 `.py`，**新建一个 `Core` 把目标文件整体执行一遍**，然后把它的结果环境包装成 `PModel` 绑定为模块对象。

## 几处设计决定（以及当时为什么这么写）

- **块帧栈的形态是为将来的 `try/except` 留的**：`break` / `continue` 用"倒着找第一个循环帧"而不是直接弹栈顶，循环结束再单独弹帧 —— 这个结构本身支持"逐层退出直到命中目标帧"，也就是异常展开需要的动作。循环控制其实用不着这么写，**这是给 `try` 留的口子**，后来没有继续做。
- **函数的初始化不在构造函数里**：`makeShared<T>()`（`core/utils/functions.h`）在创建完对象后统一调用 `obj->init()`。原因是对象在构造期间还拿不到自己的智能指针（`sharedFromThis` 拿到的是空的），所以把依赖自身指针的初始化挪到构造之后。这套约定也让 `PFunction` 的闭包环境、参数绑定都能在对象"活过来"之后再建立。
- **字节码是可序列化的**：`Instruction` 重载了 `QDataStream` 的 `<<` / `>>`，每个函数体编完就 `toByteArray()` 存进函数对象 —— 所以"代码对象"在这里就是一段字节数组，也因此具备了落盘/缓存的基础。

## 怎么跑

- **环境**：Visual Studio 2022（`python.sln`），Qt（工程用到 `core/gui/widgets`、`QJson`、`QHash`/`QMap`/`QVector`），C++。
- **入口**：`main.cpp` —— 既可以带参数指定 `.py` 直接跑，也可以起界面。
- **界面**：`MainWindow` 提供打开/编辑/保存（带临时文件 + 原子替换的保存逻辑）与运行，运行输出走 `Logger`。
- **编译产物**（`x64/`、`.vs/`）不入库。

## 开发时间线

| 时间 | 进展（取自提交信息） |
|---|---|
| 2026-03-29 | 从 Qt 版**移植**过来（"成功移植，从 qt 到此"），建立对象系统与词法分析 |
| 之后 | 类的定义/实例化/类内函数/`self` 绑定 → `__add__` 重载与属性拦截 → **继承（C3 MRO）** → `super` → 默认参数 → `pass` |
| 2026-04~07 | 把输出搬进界面（含编码问题）、简单可视化、`import` 初步实现、内置类型内置方法（`PList.append`）、全局变量 |
| 最后 | `refactor/architecture` 分支 + 打 `proto_v1` 标签，随后继续重构项目结构与语法问题 |

`master` 与 `refactor/architecture` 两条线都在仓库里，`proto_v1` 是一个阶段性标记。

## 当前状态与已知问题（诚实清单）

这是一个**边学边写、以跑通为目标**的实现，不是完整 CPython：

- **dunder 分派没有走 MRO**：`pvm.cpp` 里 `add/sub/mul/...` 找 `__add__` 时用的是 `classObj->getFunctions().find(...)`，只查本类；父类定义的运算符重载不会被调用。属性读取走的是 MRO 感知的 `PClass::__getattribute__`，两者不一致。
- **`import` 语句在语句编译分支里少一个 `break`**（`compiler.cpp` 的 `NodeType::Import` 分支），会继续落到 `default` 再编一次。
- **`Parser::throwErrorMsg()` 丢掉了传进来的信息**，统一抛 `"缺失的左括号"`，所以语法错误的提示不准确（行号与消息在调用点已经算好了，只是没被用上）。
- **默认参数只在"未传该参数"时按 `PFunction::__call__` 的规则生效**，参数绑定逻辑对"位置参数越过默认值"等边界情况的处理还在完善中。
- 字符串字面量不支持转义序列；`1.` 这类只有小数点的数字会被判为非法。
- **`input` 只到词法层**：`lexer.h` 把它列为关键字（`TokenType::INPUT`），但 parser / compiler / VM 里都没有对应处理 —— `parsePrimary` 遇到它走 `default` 分支报"未知的符号"。
- 闭包：**结构已经铺好但没跑通** —— `PFunction` 在定义处捕获环境、`Environment::getObj` 的查找链里也预留了闭包分支（已提交版本里这两处都在）。但真正让嵌套函数访问外层局部变量这件事没完成：本地未提交的修改里试过一版，帧的结构被改乱后就停在那里了。
- `try/except`：**有铺垫，没实现**。证据是帧栈的用法 —— `break` / `continue` 是**倒着遍历 `blockFrameStack` 找第一个循环帧**，而循环体结束时由 `LOOP_FOR_END` / `LOOP_WHILE_END` **弹帧**。如果这个块帧栈只服务循环，`break` 直接弹一次栈顶就够了，不必"查找 + 结束再弹"。留成"可逐层退出、直到命中目标帧"的形态，是为了将来接 `try` 的异常展开（栈展开到 handler）—— 但后续没有做。
- 装饰器、生成器、推导式等尚未涉及。

## 说明

- 仓库里保留了大量中文注释、调试输出（`log(...)` / `dumpStack`）、被注释掉的旧实现路径 —— 它们是这条学习路线的一部分，不做清理。
- 数据集、编译产物、VS/Qt 的本机配置文件已由 `.gitignore` 排除。
