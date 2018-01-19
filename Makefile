
ifeq ("$(wildcard ~/.config/farfetch/settings.ini)","")
	FF_CP_CFG = cp build/settings.ini ~/.config/farfetch/
endif

run:
	g++ main.cpp -s -std=c++11 -o build/ff

install:
	mkdir -p ~/.config/farfetch/ascii ~/bin
	$(FF_CP_CFG)
	cp build/ascii/* ~/.config/farfetch/ascii
	cp build/ff ~/bin/
	@if ! grep -q 'PATH=$$PATH:$$HOME/bin' ~/.bashrc && ! grep -q 'PATH=$$PATH:~/bin' ~/.bashrc; then echo 'export PATH=$$PATH:$$HOME/bin' >> ~/.bashrc; fi
