package examples

import org.scalatest.funsuite.FixtureAnyFunSuite
import org.scalatest.matchers.should.Matchers
import org.scalatest.{BeforeAndAfterAll, Outcome}

import com.xspcomm.{BaseDUTTrait, StringVector};

// Wrapper your DUT with stable APIs: standardize the input/output
// Stable APIs will make your TestCases more reusable
class ALUTestAPI(dut: BaseDUTTrait) {
    dut.InitClock("clock")
    def getDut(): BaseDUTTrait = {
        dut
    }
    def listPins(): StringVector = {
        dut.GetXPort().GetKeys()
    }
    def reset(): Unit = {
        dut("reset") := 1
        dut.Step(1)
        dut("reset") := 0
        dut.Step(1)
    }
    def comOp(a: Int, b: Int, o: Int, c: Int): Int = {
        dut("io_operandA") := a
        dut("io_operandB") := b
        dut("io_op") := o
        dut("io_start") := 1
        var count = 0
        while(!dut("io_done").B() && count < c) {
            dut.Step(1)
            count += 1
            dut("io_start") := 0
        }
        dut("io_result").AsInt32()
    }
    def add(a: Int, b: Int): Int = {
        comOp(a, b, 0, 3)
    }
    def sub(a: Int, b: Int): Int = {
        comOp(a, b, 1, 3)
    }
    def mul(a: Int, b: Int): Int = {
        comOp(a, b, 2, 100)
    }
    def div(a: Int, b: Int): Int = {
        comOp(a, b, 3, 100)
    }
    def finish(): Unit = {
        dut.Finish()
    }
}

// This is a test base class for multi-cycle ALU
// It provides:
// 1. A fixture for each test case
// 2. A setup and teardown method for the test suite
class MultiCycleALUTestBase extends FixtureAnyFunSuite with Matchers with BeforeAndAfterAll {
  override def beforeAll(): Unit = {
    // Custom your preparation code here
    super.beforeAll()
  }
  override def afterAll(): Unit = {
    // Custom your cleanup code here
    try {
      println("All tests finished, performing cleanup...")
      ALUUtils.getCachedDUT().finish()
    } finally {}
  }
  // Custom your fixture for each test here
  case class FixtureParam(alu: ALUTestAPI)
  override def withFixture(test: OneArgTest): Outcome = {
    val alu = ALUUtils.getCachedDUT()
    try {
      test(FixtureParam(alu))
    }finally {alu.reset()}
  }
}

// This is a utility object to cache the ALUTestAPI instance
object ALUUtils {
  var dut: ALUTestAPI = null
  def getCachedDUT(): ALUTestAPI = {
    if(dut == null) {
        dut = new ALUTestAPI(TestUtil.CreateDUT[MultiCycleALU](() => new MultiCycleALU(32)))
    }
    dut
  }
}