package examples

class AdderWithControlTest extends AdderWithControlTestBase{

  // Test cases
  test("test list ports") { fixture =>
    val adder = fixture.adder
    val pins = adder.listPins()
    println("All pins: " + pins)
    assert(pins.contains("clock"))
  }

  test("test Addition") { fixture =>
    val adder = fixture.adder
    val result = adder.add(5, 3)
    println("Addition result: " + result)
    assert(result == 8)
  }
}
