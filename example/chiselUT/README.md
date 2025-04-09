## Chisel UT 测试 (Chisel UT Testing)

#### 案例介绍 (Introduction)
picker提供Scala接口，可以把Chisel版本的 Module 转为对应的scala Trait，然后基于scalactest等Scala生态中的测试框架进行UT验证。本案例的目录结构如下 (Picker provides a Scala interface that converts Chisel Modules into corresponding Scala Traits, enabling UT verification using testing frameworks from the Scala ecosystem such as ScalaTest. The structure of this example is as follows:)：

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

1. 测试API封装，例如 (est API wrappers, such as)：
    1. AdderTestAPI.scala
    1. ALUTestAPI.scala
1. 测试用例，例如 (Test cases, such as)：
    1. AdderTestCases.scala
    1. ALUTestCases.scala
1. 测试公共基础，例如 (Common test utilities, such as)：
    1. TestUtil.scala

测试API封装的目的是让DUT与测试用例尽可能的解耦，通过保持测试API尽可能稳定的情况下，方便测试用例的复用 (The purpose of test API wrappers is to decouple the DUT from test cases as much as possible, facilitating test case reuse by keeping the test API as stable as possible.)。

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

通过上述接口，可以完成对DUT的验证。XClock，XPort，XData的操作接口可参考其[说明文档](https://github.com/XS-MLVP/xcomm/blob/master/docs/APIs.cn.md) (Using the above interface, DUT verification can be completed. For information on XClock, XPort, and XData operation interfaces, please refer to their [documentation](https://github.com/XS-MLVP/xcomm/blob/master/docs/APIs.cn.md))。
