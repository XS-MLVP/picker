default: test
.PHONY: check-picker test clean default

export PATH := ../../build/bin:${PATH}

check-picker:
	@which picker > /dev/null 2>&1 || { echo "Error: 'picker' command not found!"; exit 1; }
	@echo "find picker: `which picker`"

test: check-picker
	mill ut.test

clean:
	rm out .pickerChiselUT -rf
