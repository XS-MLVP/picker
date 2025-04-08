package examples

import org.scalatest.funsuite.FixtureAnyFunSuite
import org.scalatest.matchers.should.Matchers
import org.scalatest.{BeforeAndAfterAll, Outcome}

class MultiCycleALUTest extends FixtureAnyFunSuite with Matchers with BeforeAndAfterAll {
  // Before and after all
  override def beforeAll(): Unit = {
    super.beforeAll()
  }
  override def afterAll(): Unit = {
    try {
      println("All tests finished, performing cleanup...")
      ALUUtils.getDUT().finish()
    } finally {}
  }
  // Fixture
  case class FixtureParam(alu: ALUTestAPI)
  override def withFixture(test: OneArgTest): Outcome = {
    val alu = ALUUtils.getDUT()
    try {
      test(FixtureParam(alu))
    }finally {alu.reset()}
  }
  // Test cases
  test("test list ports") { fixture =>
    val alu = fixture.alu
    val pins = alu.listPins()
    println("All pins: " + pins)
    assert(pins.contains("clock"))
  }

  test("test Addition") { fixture =>
    val alu = fixture.alu
    val result = alu.add(5, 3)
    println("Addition result: " + result)
    assert(result == 8)
  }

  test("test Subtraction") { fixture =>
    val alu = fixture.alu
    val result = alu.sub(5, 3)
    println("Subtraction result: " + result)
    assert(result == 2)
  }

  test("test Multiplication") { fixture =>
    val alu = fixture.alu
    val result = alu.mul(5, 3)
    println("Multiplication result: " + result)
    assert(result == 15)
  }

  test("test Division") { fixture =>
    val alu = fixture.alu
    val result = alu.div(6, 3)
    println("Division result: " + result)
    assert(result == 2)
  }
}
