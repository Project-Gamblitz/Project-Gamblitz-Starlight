# TODO (Khangaroo): Make this process a lot less hacky (no, export did not work)
# See MakefileNSO

.PHONY: all clean starlight send linkerscript

S2VER ?= 552
S2VERSTR ?= 5.5.2
S2ROMTYPE ?= EU

ifeq ($(OS), Windows_NT)
	PYTHON = py -3
else
	PYTHON = python3
endif


all: starlight

starlight:
	$(MAKE) all -f MakefileNSO S2VER=$(S2VER) S2VERSTR=$(S2VERSTR)
	$(MAKE) starlight_patch_$(S2VER)/*.ips

starlight_patch_$(S2VER)/*.ips: patches/*.slpatch patches/configs/$(S2VER).config patches/maps/$(S2VER)/*.map \
								build$(S2VER)/$(shell basename $(CURDIR))$(S2VER).map scripts/genPatch.py
	@rm -f starlight_patch_$(S2VER)/*.ips
	$(PYTHON) scripts/genPatch.py $(S2VER)

send: all
	$(PYTHON) scripts/sendPatch.py $(IP) $(S2ROMTYPE) $(S2VER)

clean:
	$(MAKE) clean -f MakefileNSO
	@rm -fr starlight_patch_*

linkerscript:
	$(PYTHON) scripts/genLinkerScript.py patches/maps/551/main.map 0x2EBE000
	mv syms.ld linkerscripts/syms$(S2VER).ld