FOLDERS=TTIComp OTComp OTLegacy fonts

all:
	$(foreach f,$(FOLDERS),$(MAKE) -C $(f);)

clean:
	$(foreach f,$(FOLDERS),$(MAKE) -C $(f) clean;)

