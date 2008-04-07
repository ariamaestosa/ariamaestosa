
# this first target, 'everything' is a placeholder which makes the required subdirectories and then
# calls make again with the required directories made. Since these subdirectories are part of the
# search paths, make must see them when invoked otherwise it gets confused.

.PHONY : everything


ifeq ($(CROSS_COMPILING),1)
everything : native-dirs dirs
	@$(MAKE) all
else
everything : dirs
	@$(MAKE) all
endif

