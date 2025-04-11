## Chisel UT 测试 (Chisel UT Testing)

#### 案例介绍 (Introduction)
picker提供Scala接口，可以把Chisel版本的 Module 转为对应的scala Trait，然后基于scalactest等Scala生态中的测试框架进行UT验证。本案例的目录结构如下 (Picker provides a Scala interface that converts Chisel Modules into corresponding Scala Traits, enabling UT verification using testing frameworks from the Scala ecosystem such as ScalaTest. The structure of this example is as follows)：

```bash
chiselUT
├── Makefile     # 运行入口，支持make test 和 make clean (Entry point, supports "make test" and "make clean")

├── README.md                            # 说明文件 (Readme documentation)
├── build.sc                             # scala环境配置 (Scala environment configuration)
└── src
    ├── main
    │   └── scala                        # DUT 源代码 (DUT source code)
    │       ├── ALU.scala                # 简单ALU（支持加减乘除） (Simple ALU supports: add, subtract, multiply, divide)
    │       └── Adder.scala              # 简单加法器 (Simple adder)
    └── test
        └── scala                        # 测试（验证）代码 (Test/verification code)
            ├── AdderTestAPI.scala       # Adder 接口封装 (Adder interface wrapper)
            ├── AdderTestCases.scala     # Adder 测试用例 (Adder test cases)
            ├── ALUTestAPI.scala         # ALU   接口封装 (ALU interface wrapper)
            ├── ALUTestCases.scala       # ALU   测试用例 (ALU test cases)
            └── TestUtil.scala           # 测试公共服务 (Common test utilities)
```

在上述示例中，将测试分成了以下三部分 (In this example, testing is divided into three parts)：

1. 测试API封装，例如 (Test API wrappers, such as)：
    1. AdderTestAPI.scala
    1. ALUTestAPI.scala
1. 测试用例，例如 (Test cases, such as)：
    1. AdderTestCases.scala
    1. ALUTestCases.scala
1. 测试公共基础，例如 (Common test utilities, such as)：
    1. TestUtil.scala

测试API封装的目的是让DUT与测试用例尽可能的解耦，通过保持测试API尽可能稳定的情况下，方便测试用例的复用 (The purpose of test API wrappers is to decouple the DUT from test cases as much as possible, facilitating test case reuse by keeping the test API as stable as possible)。

#### API介绍 (API Introduction)

xcomm为Chisel提供了设计module到scala Trait的转换，具体如下 (xcomm provides conversion from Chisel module designs to Scala Traits, specifically)：

**generateDUT** ：
通用DUT生成生成方法如下 (The general DUT generation method is as follows)：
```scala
package com.xspcomm

object chiselUT {
    def generateDUT[A: TypeTag](                           // 需要转换的Chisel 类 (Chisel class to be converted)
                  issueVerilog: (String, String) => Unit,  // 具体chisel到verilog转换的回调函数 (Callback function for Chisel to Verilog conversion)
                  workDir: String = ".pickerChiselUT",     // 生成jar包的工作目录 (Working directory for generating JAR packages)
                  forceRebuild: Boolean = false,           // 是否强制重新编译 (Whether to force recompilation)
                  autoRebuild: Boolean = true,             // 是否自动检查需要重新编译 (Whether to automatically check if recompilation is needed)
                  pickerExArgs: String = "",               // 自定义picker转换参数 (Custom picker conversion parameters)
                  ): BaseDUTTrait                          // 返回类型 (Return type)
}
```

在本案例中，generateDUT的调用位于`src/test/scala/TestUtil.scala`，该方法返回的是原生scala对象，可以现有scala生态进行集成 (In this example, the example of using generateDUT is located in `src/test/scala/TestUtil.scala`. This method returns a native Scala object that can be integrated with the existing Scala ecosystem)。

**BaseDUTTrait** ：
通用DUT操作接口`BaseDUTTrait`的定义与说明如下 (The definition and description of the general DUT operation interface `BaseDUTTrait` is as follows)：

```scala
trait BaseDUTTrait{
    def GetXClock(): XClock                         // 获取时钟 (Get clock)
    def GetXPort(): XPort                           // 获取引脚Port (Get pin Port)
    def OpenWaveform(): Boolean                     // 开启波形 (需要参数开启) (Enable waveform,requires parameter setting)
    def CloseWaveform(): Boolean                    // 关闭波形 (Close waveform)
    def SetWaveform(wave_name: String): Unit        // 开启波形并设置波形文件名 (Enable waveform and set waveform filename)
    def FlushWaveform(): Unit                       // 刷新波形缓存 (Flush waveform cache)
    def SetCoverage(coverage_name: String): Unit    // 设置覆盖率文件 (需要参数开启) (Set coverage file, requires parameter setting)
    def Step(i: Int = 1): Unit                      // 执行i个时钟周期 (Execute i clock cycles)
    def StepRis(callback: (Long) => Unit): Unit     // 设置上升沿回调 (Set rising edge callback)
    def StepFal(callback: (Long) => Unit): Unit     // 设置下降沿回调 (Set falling edge callback)
    def Finish(): Unit                              // 完成并释放DUT (Complete and release DUT)
    def InitClock(clock_name: String): Unit         // 初始化时钟引脚 (Initialize clock pin)
    def RefreshComb(): Unit                         // 刷新组合电路 (Refresh combinational circuit)
    def CheckPoint(check_point: String): Unit       // 保持电路状态到文件 (Save circuit state to file)
    def Restore(check_point: String): Unit          // 从文件恢复电路状态 (Restore circuit state from file)
    // 通过VPI列出内部信号 (需要参数开启) (List internal signals via VPI, requires parameter setting)
    def VPIInternalSignalList(prefix: String = "", deep: Int = 99): Array[String]
    // 获取单个内部信号 (需要参数开启) (Get a single internal signal, requires parameter setting)
    def GetInternalSignal(name: String, index: Int = -1, use_vpi: Boolean = false): XData
    // 获取数组内部信号 (需要参数开启) (Get array internal signals, requires parameter setting)
    def GetInternalSignal(name: String, is_array: Boolean): Array[XData]\
    // 获取内部信号列表 (Get internal signal list)
    def GetInternalSignalList(prefix: String="", deep: Int = 99, use_vpi: Boolean = false): Array[String]
    // 等价于 GetXPort().Get(key)，方便信号访问 (Equivalent to GetXPort().Get(key), convenient signal access)
    def apply(key: String): XData
}
```

**nameOf**：

获取scala变量名，接受不定参，返回对应`Map[String, T]`（Retrieve Scala variable names, accept variadic arguments, and return a corresponding `Map[String, T]`）：
```scala
object nameOf {
  def apply(exprs: Any*): Map[String, Any]
}
// 使用举例（Example usage）:
var ret = nameOf(a, b)
/*
ret = {
    "a" -> a,
    "b" -> b,
}
*/

```

**attrOf**：

获取scala实例中所有类型为T的var或者val，返回`Map[String, T]`，支持自定义过滤（Retrieve all `var` or `val` fields of type `T` from a Scala instance and return them as a `Map[String, T]`, with support for custom filtering）：
```scala
object attrOf {
  def apply[T: TypeTag](x: AnyRef, filter: (String, T)=>Boolean = (k:String, v:T) => true): Map[String, T]
}

// 使用举例（Example usage）:
// 获取当前class中类型属于chisel3.Data的所有val和var（Retrieve all `val` and `var` fields of type `chisel3.Data` in the current class）
var ret = attrOf[chisel3.Data](this,
                               filter = (k, v) => !v.isLit && // Data不能是字面量（Data must not be a literal）
                                                  !v.isInstanceOf[chisel3.Bundle] // Data不能是Bundle类型（Data must not be of type Bundle）
                               )

```

**保留内部信号（Preserve Internal Signals）**：

保留指定Reg或者Wire，防止被chisel或者verilator优化掉，以便在TestCase中获取进行读取。具体可参考`src/test/scala/TestUtil.scala`中`MarkAsDebug`的实现（Preserve specified `Reg` or `Wire` to prevent them from being optimized out by Chisel or Verilator, allowing them to be accessed and read in test cases. Refer to the implementation of `MarkAsDebug` in TestUtil.scala for details）：

```scala
// src/main/scala/ALU.scala:87

...
  // 在 DUT 定义的最后标记需要保留，防止被优化掉的Reg或者Wire
  //（At the end of the DUT definition, mark the registers or wires that need to be preserved to prevent optimization）
  MarkAsDebug(nameOf(unusedReg, unusedWire1, unusedWire2))
  // 或者通过attrOf把所有信号都保留，Lit和Bundle除外
  // Or use `attrOf` to preserve all signals, excluding literals and Bundles.
  //MarkAsDebug(attrOf[chisel3.Data](this, filter = (k, v) => !v.isLit && !v.isInstanceOf[chisel3.Bundle]))
}
```

上述`MarkAsDebug`的作用是保留，正常编译情况下会被优化掉的内部信号，例如`ALU.scala`文件中的`unusedReg, unusedWire1, unusedWire2`。保留的信号会在名称后加上`_debug`后缀，例如（The purpose of `MarkAsDebug` is to preserve internal signals that would normally be optimized out during compilation, such as `unusedReg`, `unusedWire1`, and `unusedWire2` in the `ALU.scala` file. The preserved signals will have a `_debug` suffix added to their names, for example）：
```scala
// 通过unusedReg_debug访问原始 unusedReg（Access the original unusedReg via unusedReg_debug）
dut("MultiCycleALU_top.MultiCycleALU.unusedReg_debug").AsInt32()
```

MarkAsDebug可通过环境变量`XDebug`控制是否生效，例如（`MarkAsDebug` can be controlled via the `XDebug` environment variable, for example）：
```bash
# 关闭Debug，不对MarkAsDebug的信号做任何处理
#（Disable Debug, do not process signals in MarkAsDebug）
make XDebug=0
# 开启Debug，利用 "名称+_debug"的Reg保留MarkAsDebug中信号的值
#（Enable Debug, preserve the values of signals in MarkAsDebug using "name + _debug" registers）
make XDebug=1
```

通过上述接口，可以完成对DUT的验证。XClock，XPort，XData的操作接口可参考其[说明文档](https://github.com/XS-MLVP/xcomm/blob/master/docs/APIs.cn.md) (Using the above interface, DUT verification can be completed. For information on XClock, XPort, and XData operation interfaces, please refer to their [documentation](https://github.com/XS-MLVP/xcomm/blob/master/docs/APIs.cn.md))。
