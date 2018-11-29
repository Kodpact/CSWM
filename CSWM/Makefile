edit:
	g++ -shared -mtune=i386 -O3 -m32 \
	-I ./ \
	-I ../ \
	-I ../SDK/ \
	-I ../SDK/metamod/ \
	-I ../SDK/cssdk \
	-D HAVE_STDINT_H \
	-D _CRT_SECURE_NO_WARNINGS \
	182/AMXX.cpp Module.cpp CSWM.cpp HEFW.cpp FakeCMD.cpp \
	-o cswm_amxx_i386.so