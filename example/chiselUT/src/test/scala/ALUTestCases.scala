package examples

class MultiCycleALUTest extends MultiCycleALUTestBase{

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
