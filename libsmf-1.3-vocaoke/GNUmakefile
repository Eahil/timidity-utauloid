#this makefile is handwritten, and should be kept short

default:
	@make -s -C build
%:
	@echo make -s -C build $@
distclean:
	@rm -r build
test: 
	@make -s -C build
	rm -f /tmp/autosave.mid
	./build/src/vsqdump ~/UTAU/vsq/ievan.vsq
	./build/src/smfsh /tmp/autosave.mid
	

