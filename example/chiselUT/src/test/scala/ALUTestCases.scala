package examples

class MultiCycleALUTest extends MultiCycleALUTestBase{

  // Test cases
  test("test list ports") { fixture =>
    val alu = fixture.alu
    val pins = alu.listPins()
    println("All pins: " + pins)
    assert(pins.contains("clock"))
  }

  test("test list inner signals") { fixture =>
    val alu = fixture.alu
    val pins = alu.listInnerSignals()
    println("All inner signals: " + pins.mkString(", "))
    assert(pins.contains("MultiCycleALU_top.reset"))
  }

  test("test Addition") { fixture =>
    val alu = fixture.alu
    val result = alu.add(5, 3)
    println("Addition result: " + result)
    val aluState = alu.getDut().GetInternalSignal("MultiCycleALU_top.MultiCycleALU.state")
    println("ALU state: " + aluState.U().intValue)
    assert(result == 8)
    assert(aluState.U().intValue == 4)
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
