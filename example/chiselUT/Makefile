default: test
.PHONY: check-picker

export PATH := ../../build/bin:${PATH}

check-picker:
    @which picker > /dev/null 2>&1 || { echo "Error: 'picker' command not found!"; exit 1; }

test:
	mill ut.test

clean:
	rm out .pickerChiselUT -rf
