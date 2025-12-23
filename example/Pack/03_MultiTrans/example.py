#!/usr/bin/env python3
# File       : test_alu.py
# Description: ALU test with bidirectional communication
#              - Python sends operations via alu_op
#              - UVM processes and returns results via alu_result
#              - Python verifies results

import sys
import os

# Ensure current directory is in path for module import
sys.path.insert(0, '.')

from ALU import DUTALU


class ALUChecker:
    """ALU result checker"""
    
    @staticmethod
    def calculate_expected(opcode, operand_a, operand_b):
        """
        Calculate expected ALU result based on operation.
        
        Args:
            opcode: 0=ADD, 1=SUB, 2=MUL, 3=DIV
            operand_a: First operand
            operand_b: Second operand
            
        Returns:
            (result, status) tuple
        """
        status = 0  # bit 0: overflow, bit 1: zero, bit 2: negative
        
        if opcode == 0:  # ADD
            result = operand_a + operand_b
            if result > 0xFFFF:
                status |= 0x1  # overflow
                result &= 0xFFFF
        elif opcode == 1:  # SUB
            result = operand_a - operand_b
            if result < 0:
                status |= 0x4  # negative
                result = (-result) & 0xFFFF
        elif opcode == 2:  # MUL
            result = operand_a * operand_b
            if result > 0xFFFF:
                status |= 0x1  # overflow
                result &= 0xFFFF
        elif opcode == 3:  # DIV
            if operand_b == 0:
                result = 0
                status |= 0x1  # overflow (division by zero)
            else:
                result = operand_a // operand_b
        else:
            result = 0
            
        if result == 0:
            status |= 0x2  # zero flag
            
        return result & 0xFFFF, status & 0x7


def monitor_callback(dut):
    """Callback to display monitor updates"""
    print(f"  [Monitor] op={dut.opcode.value}, a={dut.operand_a.value}, "
          f"b={dut.operand_b.value}, result={dut.result.value}, status=0x{dut.status.value:X}")


def test_alu_operations():
    """Test ALU with various operations"""
    print("\n" + "="*80)
    print(" ALU Multi-Transaction Test ".center(80))
    print("="*80 + "\n")
    
    # Initialize DUT
    print("Initializing DUT...")
    dut = DUTALU()
    dut.SetUpdateCallback(monitor_callback)
    print("✓ DUT initialized\n")
    
    # Test cases: (opcode, operand_a, operand_b, description)
    test_cases = [
        (0, 10, 20, "ADD: 10 + 20 = 30"),
        (0, 255, 255, "ADD: 255 + 255 = 510 (overflow)"),
        (1, 50, 30, "SUB: 50 - 30 = 20"),
        (1, 10, 20, "SUB: 10 - 20 = -10 (negative)"),
        (2, 5, 6, "MUL: 5 * 6 = 30"),
        (2, 200, 200, "MUL: 200 * 200 = 40000 (overflow)"),
        (3, 100, 5, "DIV: 100 / 5 = 20"),
        (3, 10, 0, "DIV: 10 / 0 = error"),
    ]
    
    print("-" * 80)
    print("Running ALU Test Cases")
    print("-" * 80)
    
    passed = 0
    failed = 0
    
    for i, (opcode, operand_a, operand_b, description) in enumerate(test_cases, 1):
        print(f"\nTest {i}: {description}")
        
        # Set operation inputs
        dut.opcode.value = opcode
        dut.operand_a.value = operand_a
        dut.operand_b.value = operand_b
        
        # Initial result values (will be overwritten by UVM)
        dut.result.value = 0
        dut.status.value = 0
        
        print(f"  Sending: opcode={opcode}, a={operand_a}, b={operand_b}")
        
        # Step to send operation and receive result
        # Need enough cycles for: send alu_op -> UVM process -> receive alu_result
        dut.Step(1)
        
        # Get actual result from DUT (updated by monitor)
        actual_result = dut.result.value
        actual_status = dut.status.value
        
        # Calculate expected result
        expected_result, expected_status = ALUChecker.calculate_expected(
            opcode, operand_a, operand_b
        )
        
        # Verify
        print(f"  Expected: result={expected_result}, status=0x{expected_status:X}")
        print(f"  Actual:   result={actual_result}, status=0x{actual_status:X}")
        
        if actual_result == expected_result and actual_status == expected_status:
            print(f"  ✓ PASS")
            passed += 1
        else:
            print(f"  ✗ FAIL - Result or status mismatch!")
            failed += 1
    
    # Summary
    print("\n" + "="*80)
    print(f"Test Summary: {passed} passed, {failed} failed out of {len(test_cases)} tests")
    print("="*80 + "\n")
    
    if failed > 0:
        sys.exit(1)
    else:
        print("✓ All tests passed!")
        sys.exit(0)


if __name__ == "__main__":
    test_alu_operations()
