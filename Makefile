CFLAGS += -O3 -g -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0

.PHONY: all
all: GZIP NCOM BC YPSV CVS POLY

.PHONY: GZIP, RUN-GZIP
GZIP:
	git clean -xfd gzip-1.2.4; git restore gzip-1.2.4
	cd gzip-1.2.4; CFLAGS="$(CFLAGS) -std=gnu90" bash configure; make
RUN-GZIP: GZIP
	./gzip-1.2.4/gzip _input/GZIP/3DiI39GpdzhhZ28vdkqtr38bfzHnettJ07gPAr96ppKzVx725aTRfFANoiu9ZHEJqmN1Xmic6UVudmW36VXmOy3dKbXqHhl7vd57GnWYLvSEZwUbyLuPE5oIZzHd26Tp/M7g4gkbAjKlDXwRVyVHZW1KwJyKIRtKbCeOrRJmFVkWaFDPNNhaveNvHxNRNG7cRvZ3jE0egyMDWwZBjKy6uRd25slx9BkDtl9aekCpP7fFwD2QwlLRbJO15QuZARDmZ/X7exgvSH5k0kdvlpfMV9Tzfi8lqx6VGbELKYtsIfJJL83L7ZjG2UgFKU5GABkLX2uRdbwTPJ0oWzbdBDrfM67ZTrxjKokpR2E2UDFtFJclvbJeMNkZXblI6BeGkGsxno/H3YwEsRbTbgvrBnl2VYGPPidm2j6SpGKkgtWSCYTdXxmgzDToa6Hf9jQoRFbJEzvDGWpBRpNyIAEPUG0uChg2ojFWdQfWi63YvxKpR3JX1bOWPnnwbX0vQuIQK37ksDY/7avp28eABSTAaBFjlTZ28bVmLN8W3eo8V4A0rmFMwRdrKpKz2X45WwRM05TdjAlHvFzOSR4xlxOsaappoxENXzyfWEAMsc6sSR0PqP2zPkm5EAiAhYnjG5ZWgUCCWflc/svTd5qnR9w1rP0BNEDTcuPKqrOnAFVvOD8ZPPokYeG7FnD2TAERb95w0oDCAEifd7bJp0FsVa6zW9nXrkydf2BFx9Jqq0OXNNgcqIK738C4vDiQFZ1itf1tMymOXAnIP/KMOTYKfAA7P4pmIGicRIp52dPftGzldJTK7Mak3THRgP5uI2Y6Oi3zp0wvr7Hp1Tcdg0bmayRaKD8w5QwgIU0a5XuTmgeEK6nIJZHLOIYdscfJMi8fOQVE35jbaZGkty/Cd0jSLffnNowozhZMU089zqv1fqGGxGys1VQrkTMff9BzNV7kMylj9SO6c3GERd1I5buKA5Z3jHnu9SJiDf2emndJ5tWebDKoVh0KzMMK38FKSsMbSfUAS8oOP2svQ9g

.PHONY: NCOM, RUN-NCOM
NCOM:
	git clean -xfd ncompress-4.2.4; git restore ncompress-4.2.4
	cd ncompress-4.2.4; cp Makefile.def Makefile; make options="$(CFLAGS) -std=gnu90 -DDIRENT=1 -DUSERMEM=800000 -DREGISTERS=3 -DNOFUNCDEF=1"
RUN-NCOM: NCOM
	rm -f _input/NCOM/pjEgA16x4jl0Tp8RYDXUvipsb0VKc3ZqFXx1WWzVoNcddnrOsJaa3LR5Ug1UuyXo5tjSFlccGbOVe5eKrSWy4eYbkbAQbDFP1K4wMW2eOuiAuxhG0W217uadhtos6HBW/lY9oIX4xOiTIGRWRXkxmVQGjCAwEuKcczNpE60MTYfXwLSpa88EYBEqOvPy5C0FmBaF7THs4dFRBFncMx25ctvW1lD4k5YGP6SfquRZAZ7iUEIJxuFKqgvtedvRIoOwx/iqcFaw5mNmKDSrGg83G8hvwIWYZ6A3zjoAEzKfRFPR36hnAAwgttWRJGLfSIHayBrCpB5zH721SeFht2WKQyfQK92B0tpfARQNQX8WxWy19RCv0CbzQr8K8mtArIozBT/ymU3gtwIv1DXim0tq9XiUhhBZxYffOE7wEyXsnJ0oEYssAG2VMg4RsurLB2zeKKbM55WF2MmUtPZYCEfIm38sLdIJQnjzIKa0ImLX5vqGjZFtM8pJp9NmlKYqnDNjBlF/az6qYyWY4kCl3SEOc6sOkswU4zEgNN4ojzukBTdvabNi7AKRqpgLkWBzyMSp5UEs4cQY6lPmqg9RBeR3Y8SzIO27JgtKk01K8GMdz4kHeyQNoRglCcYmOGgPmdVdyJNS/QhyZNL5wHL3lZ6qO5Bxdz82VHGSO7IffDAmsgYixjCHsbiSVNjGtlLkOLhIFuYeaym7Ne8CnHbVRZz7Hqif1N3GdS9wzBEBD1XCVicbPSmA3nqwhn7tgXPiISxn6V0sC/xH4Wa6FCqkWRpK3qGmUlyyRFuMWzKN0Np1WlRf0EKH43X8Vhv9BLeTg8CxNlgieUdGDroMgtd2KAmO5pMlLEwynHrwSJP9qrhZKeJBaIw47WnQwOJML4ZT33Sdvoyh0B/woeaoJTq3O4JZiOZ7bjAgxvwhIH6kQjbNSdexj3bOIt5ConGZ6lGNdo6GTA1ovHI42nrAYPucrbwPEizuhXvM1H6xpsnnsxKeKt2sGOOmA8FgowKn4pEkyfsiVTM2Sac.Z
	./ncompress-4.2.4/compress _input/NCOM/pjEgA16x4jl0Tp8RYDXUvipsb0VKc3ZqFXx1WWzVoNcddnrOsJaa3LR5Ug1UuyXo5tjSFlccGbOVe5eKrSWy4eYbkbAQbDFP1K4wMW2eOuiAuxhG0W217uadhtos6HBW/lY9oIX4xOiTIGRWRXkxmVQGjCAwEuKcczNpE60MTYfXwLSpa88EYBEqOvPy5C0FmBaF7THs4dFRBFncMx25ctvW1lD4k5YGP6SfquRZAZ7iUEIJxuFKqgvtedvRIoOwx/iqcFaw5mNmKDSrGg83G8hvwIWYZ6A3zjoAEzKfRFPR36hnAAwgttWRJGLfSIHayBrCpB5zH721SeFht2WKQyfQK92B0tpfARQNQX8WxWy19RCv0CbzQr8K8mtArIozBT/ymU3gtwIv1DXim0tq9XiUhhBZxYffOE7wEyXsnJ0oEYssAG2VMg4RsurLB2zeKKbM55WF2MmUtPZYCEfIm38sLdIJQnjzIKa0ImLX5vqGjZFtM8pJp9NmlKYqnDNjBlF/az6qYyWY4kCl3SEOc6sOkswU4zEgNN4ojzukBTdvabNi7AKRqpgLkWBzyMSp5UEs4cQY6lPmqg9RBeR3Y8SzIO27JgtKk01K8GMdz4kHeyQNoRglCcYmOGgPmdVdyJNS/QhyZNL5wHL3lZ6qO5Bxdz82VHGSO7IffDAmsgYixjCHsbiSVNjGtlLkOLhIFuYeaym7Ne8CnHbVRZz7Hqif1N3GdS9wzBEBD1XCVicbPSmA3nqwhn7tgXPiISxn6V0sC/xH4Wa6FCqkWRpK3qGmUlyyRFuMWzKN0Np1WlRf0EKH43X8Vhv9BLeTg8CxNlgieUdGDroMgtd2KAmO5pMlLEwynHrwSJP9qrhZKeJBaIw47WnQwOJML4ZT33Sdvoyh0B/woeaoJTq3O4JZiOZ7bjAgxvwhIH6kQjbNSdexj3bOIt5ConGZ6lGNdo6GTA1ovHI42nrAYPucrbwPEizuhXvM1H6xpsnnsxKeKt2sGOOmA8FgowKn4pEkyfsiVTM2Sac

.PHONY: BC, RUN-BC
BC:
	git clean -xfd bc-1.06; git restore bc-1.06
	cd bc-1.06; aclocal; automake --add-missing; CFLAGS="$(CFLAGS)" bash configure; make
RUN-BC: BC
	./bc-1.06/bc/bc ./_input/BC/bad.b

.PHONY: YPSV, RUN-YPSV
YPSV:
	git clean -xfd ypserv-2.2; git restore ypserv-2.2
	cd ypserv-2.2; autoreconf -i; CFLAGS="-I/usr/include/tirpc $(CFLAGS)" LIBS="-ltirpc" bash configure; make
RUN-YPSV: YPSV
	

.PHONY: CVS, RUN-CVS
CVS:
	git clean -xfd cvs-1.11.4; git restore cvs-1.11.4
	cd cvs-1.11.4; bash noautomake.sh --noautoconf; CFLAGS="$(CFLAGS) -m32" bash configure; make
RUN-CVS: CVS

.PHONY: POLY, RUN-POLY
POLY:
	git clean -xfd polymorph-0.4.0; git restore polymorph-0.4.0
	cd polymorph-0.4.0; autoupdate; aclocal; autoconf; automake --add-missing; CFLAGS="$(CFLAGS)" bash configure; make
RUN-POLY: POLY
	./polymorph-0.4.0/polymorph -f AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

.PHONY: clean
clean:
	git clean -xfd .; git restore .