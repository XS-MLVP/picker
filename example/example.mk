EXAMPLE_OUT_ROOT ?= output
EXAMPLE_LANG ?= python

EXAMPLE_VERILATOR_TARGETS := \
	Adder \
	AdderMultiInstance \
	Cache \
	CacheSignalCFG \
	DualPortStackCb \
	InternalSignals \
	MultiClock \
	RandomGenerator \
	XDummyCache \
	e203_ifu_ift2icb

EXAMPLE_TEST_ALL := Adder RandomGenerator AdderMultiInstance Cache
EXAMPLE_TEST_ALL_JAVA := Adder RandomGenerator DualPortStackCb InternalSignals CacheSignalCFG
EXAMPLE_TEST_ALL_SCALA := Adder RandomGenerator DualPortStackCb InternalSignals CacheSignalCFG

.PHONY: $(addprefix test_,$(EXAMPLE_VERILATOR_TARGETS)) \
	$(addprefix test_vcs_,$(EXAMPLE_VERILATOR_TARGETS)) \
	$(addprefix clean_,$(EXAMPLE_VERILATOR_TARGETS))

define EXAMPLE_RULES
test_$(1):
	@script="example/$(1)/release-verilator.sh"; \
	if [ ! -f "$$$$script" ]; then \
		echo "Unknown example '$(1)' or missing $$$$script"; \
		exit 1; \
	fi; \
	lang_args=""; \
	if [ -n "$(EXAMPLE_LANG)" ]; then \
		lang_args="--lang $(EXAMPLE_LANG)"; \
	fi; \
	OUT_ROOT="$(EXAMPLE_OUT_ROOT)" bash "$$$$script" $$$$lang_args $(ARGS)

test_vcs_$(1):
	@script="example/$(1)/release-vcs.sh"; \
	if [ ! -f "$$$$script" ]; then \
		echo "Example '$(1)' does not provide VCS flow: $$$$script"; \
		exit 1; \
	fi; \
	lang_args=""; \
	if [ -n "$(EXAMPLE_LANG)" ]; then \
		lang_args="--lang $(EXAMPLE_LANG)"; \
	fi; \
	GLIBC_TUNABLES=glibc.rtld.optional_static_tls=262144 OUT_ROOT="$(EXAMPLE_OUT_ROOT)" bash "$$$$script" $$$$lang_args $(ARGS)

clean_$(1):
	rm -rf $(EXAMPLE_OUT_ROOT)/$(1)
endef

$(foreach example,$(EXAMPLE_VERILATOR_TARGETS),$(eval $(call EXAMPLE_RULES,$(example))))
