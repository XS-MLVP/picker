
## DUT验证API和一般驱动流程

在DUT中，有XData、XClock、XPort 三类成员变量，如需要调用对应功能可参考：[https://github.com/XS-MLVP/xcomm/blob/master/docs/APIs.cn.md](https://github.com/XS-MLVP/xcomm/blob/master/docs/APIs.cn.md)

### DUT验证APIs

在DUT对应的Class中，以**大驼峰命名**的函数为用户需要使用的常用函数，其他命名方式的函数为内部函数，不建议用户使用。

|编号|API名称|参数|返回值|说明|
|-|--|---|---|---|
|1|InitClock(name)|字符类型，时钟引脚，或引脚的名称|-|初始化时钟，让DUT中的`XClock`绑定对应的引脚|
|2|Step(i)|int类型，时钟周期个数|-|推进时序电路i个时钟周期|
|3|StepRis(callback)|函数类型，回调函数|-|设置上升沿触发的回调函数|
|4|StepFal(callback)|函数类型，回调函数|-|设置下降沿触发的回调函数|
|5|SetWaveform(file)|字符类型，文件名，含路径|-|设置波形输出的文件|
|6|FlushWaveform()|-|-|将波形内容立即刷新到文件中，**频繁调用可能导致FST压缩能力失效**|
|7|SetCoverage(file)|字符类型，文件名，含路径|-|设置覆盖率输出的文件|
|8|CheckPoint(file)|字符类型，文件名，含路径|int类型，当前时钟周期|保存当前仿真状态到文件|
|9|Restore(file)|字符类型，文件名，含路径|int类型，保存时的时钟周期|从文件中恢复仿真状态|
|10|GetInternalSignal(name)|字符类型，信号名称|XData类型|通过变量名获取的内部信号对象，需要开启 `--vpi`，**影响性能**|
|11|VPIInternalSignalList(prefix, deep)|字符类型，前缀，深度层级|int类型，信号名称列表|获取内部信号列表，需要开启 `--vpi`|
|12|Finish()|-|-|结束仿真，保持波形，覆盖率等结果文件|
|13|RefreshComb()|-|-|推进组合电路|


#### C++ 
```c++
void InitClock(std::string name);
void Step(int i = 1);
void StepRis(std::function<void(uint64_t, void*)>, void*args=nullptr);
void StepFal(std::function<void(uint64_t, void*)>, void*args=nullptr);
void SetWaveform(std::string filename);
void FlushWaveform();
void SetCoverage(std::string filename);
int CheckPoint(const std::string filename);
int Restore(const std::string filename);
XData GetInternalSignal(const std::string name);
std::vector<std::string> VPIInternalSignalList(std::string prefix="", int deep=99);
void Finish();
void RefreshComb();
```


#### Python
```python
InitClock(name: str)
Step(i:int = 1)
StepRis(callback: Callable, args=None,  args=(), kwargs={})
StepFal(callback: Callable, args=None,  args=(), kwargs={})
SetWaveform(filename)
FlushWaveform()
SetCoverage(filename)
CheckPoint(filename: str) -> int
Restore(filename: str) -> int
GetInternalSignal(name: str) -> XData
VPIInternalSignalList(self, prefix="", deep=99) -> List[str]
Finish()
RefreshComb()
```

对于 StepRis 和 StepFal 中的回调函数，第一个参数为cycle，C++支持一个自定义参数 void*，python支持args 和 kwargs 自定义参数。其他编程语言不支持自定义参数：

```java
// java
StepRis(Consumer<Long> callback)
StepFal(Consumer<Long> callback)
// scala
StepRis(callback: (Long) => Unit)
StepFal(callback: (Long) => Unit)
// golang
StepRis(callback: func(uint64))
StepFal(callback: func(uint64))
```

**对于其他API，Java/Scala/Golang等语言类似，此处不再重复**


### 驱动DUT的一般流程

1. 创建DUT
1. 初始化时钟（如果有）
1. reset电路（如果需要reset）
1. 给DUT输入引脚写入数据
1. 驱动电路：时序电路用Step，组合电路用RefreshComb
1. 获取DUT输出

对应伪代码如下(以Python为例)：

```python
import random
from UT_DUT import *

# 创建
dut = DUT()

# 初始化
dut.SetWaveform("test.fst")
dut.InitClock("clock")

dut.reset = 1
dut.Step(1)
dut.reset = 0
dut.Step(1)

# 输入数据
dut.input_pin1.value = 123
dut.input_pin2.value = "0x456"
dut.input_pin3.value = "0b1011"

# 驱动电路 
dut.Step(1) # or dut.RefreshComb()

# 得到结果
x = dut.output_pin.value
print("result:", x)

# 销毁
dut.Finish()
```


#### DUT引脚取值与赋值

所有支持的编程语言都能提过Set(value)进行赋值，通过U()或者S()进行取值，提过At(i)获取第i位的值。

```c++
// Set(value), U(), S() 接口 c++、java、python、golang、scala 通用
// 由于Java、Scala、Golang支持bigInt类型，其U(),S()返回的也是bigInt类型，如果
// 需要类似C++返回64位数，需要用 U64() 和 S64()
dut.pin.Set(0x123)
dut.pin.Set("0b100101")
x = dut.pin.S()  // 获取有符号数
x = dut.pin.U()  // 获取无符号数
x = dut.pin.At(12).Get()  // 获取第12位
dut.pin.At(12).Set(1)     // 对第12位进行赋值
```

**注1：** 对于Golang，其pin的成员名称首字母将自动转为大写，eg: dut.Pin.Set(0x123)

**注2：** 如果pin的名称与dut中的成员变量或方法名称冲突，picker会自动加上pin_（对应Golang前缀为Pin_）前缀。冲突名称有："xclock", "xport", "dut", "initclock", "step", "stepris", "stepfal", "setwaveform", "setcoverage", "finish", "refreshcomb"等

由于C++支持运算符重载，因此在赋值和取值上可以进行简化：
```c++
dut.pin = 0x123
dut.pin = "0b100101"
auto x = dut.pin
auto x dut.pin[12]
dut.pin[12] = 1
```

python对属性value进行了重载：
```python
dut.pin.value = 0x123
dut.pin.value = "0b100101"
x = dut.pin.value         # 默认为无符号数
x = dut.pin[12]
dut.pin[12] = 1
```

scala 对运算符":="进行了重载，重载后如下

```scala
dut.pin := 0x123
dut.pin := "0b100101"
```
