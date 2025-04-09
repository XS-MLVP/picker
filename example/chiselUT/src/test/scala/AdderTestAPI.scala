package examples

import org.scalatest.funsuite.FixtureAnyFunSuite
import org.scalatest.matchers.should.Matchers
import org.scalatest.{BeforeAndAfterAll, Outcome}

import com.xspcomm.{BaseDUTTrait, StringVector};

// Wrapper your DUT with stable APIs: standardize the input/output
// Stable APIs will make your TestCases more reusable
class AdderTestAPI(dut: BaseDUTTrait) {
    dut.InitClock("clock")
    def getDut(): BaseDUTTrait = {
        dut
    }
    def listPins(): StringVector = {
        dut.GetXPort().GetKeys()
    }
    def add(a: Int, b: Int): Int = {
        dut("io_a") := a
        dut("io_b") := b
        dut("io_start") := 1
        while (!dut("io_done").B()) {
            dut.Step(1)
            dut("io_start") := 0
        }
        return dut("io_sum").AsInt32()
    }
    def reset(): Unit = {
        dut("reset") := 1
        dut.Step(1)
        dut("reset") := 0
        dut.Step(1)
    }
    def finish(): Unit = {
        dut.Finish()
    }
}

// This is a test base class for Adder
// It provides:
// 1. A fixture for each test case
// 2. A setup and teardown method for the test suite
class AdderWithControlTestBase extends FixtureAnyFunSuite with Matchers with BeforeAndAfterAll {
  override def beforeAll(): Unit = {
    // Custom your preparation code here
    super.beforeAll()
  }
  override def afterAll(): Unit = {
    // Custom your cleanup code here
    try {
      println("All tests finished, performing cleanup...")
      AdderUtils.getCachedDUT().finish()
    } finally {}
  }
  // Custom your fixture for each test here
  case class FixtureParam(adder: AdderTestAPI)
  override def withFixture(test: OneArgTest): Outcome = {
    val adder = AdderUtils.getCachedDUT()
    try {
      test(FixtureParam(adder))
    }finally {adder.reset()}
  }
}

// This is a utility object to cache the AdderTestAPI instance
object AdderUtils {
  var dut: AdderTestAPI = null
  def getCachedDUT(): AdderTestAPI = {
    if(dut == null) {
        dut = new AdderTestAPI(TestUtil.CreateDUT[AdderWithControl](() => new AdderWithControl(32)))
    }
    dut
  }
}