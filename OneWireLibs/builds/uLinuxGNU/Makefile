CC = $(GCC_PREFIX)gcc
AR = $(GCC_PREFIX)ar

# directories
PRE    = ../..
APPS   = $(PRE)/apps
COMMON = $(PRE)/common
LINK   = $(PRE)/lib/userial/link/Linux
SHARED = $(PRE)/lib/userial/shared

CFLAGS = -I $(COMMON) -I $(SHARED) -I $(LINK) -DDEBUG -c
LFLAGS = -I $(COMMON) -DDEBUG -g -o $@

ONEWIREOBJS = obj/owlinkobjs
WEATHEROBJS = obj/weather.o obj/atod20.o obj/cnt1d.o obj/swt12.o \
	obj/temp10.o $(ONEWIREOBJS)
MEMOBJS = obj/mbappreg.o obj/mbeprom.o obj/mbnvcrc.o obj/mbscrcrc.o \
	obj/mbscrex.o obj/mbshaee.o obj/mbee.o obj/mbee77.o obj/mbnv.o \
	obj/mbscr.o obj/mbscree.o obj/mbscrx77.o obj/mbsha.o obj/pw77.o \
	obj/rawmem.o obj/mbeewp.o $(ONEWIREOBJS)
FILEOBJS = obj/owfile.o obj/owpgrw.o obj/owprgm.o obj/owcache.o $(MEMOBJS)
SHAOBJS = obj/sha18.o obj/sha33.o obj/shaib.o $(FILEOBJS)
PROGS = atodtst counter coupler fish gethumd memutil mweather shaapp swtloop \
	swtsngl temp thermodl thermoms tstfind debit debitvm humalog initcopr \
	initcoprvm initrov initrovvm ps_check ps_init \
	sha_chck sha_init tm_check tm_init swtduo swtmain1c swtmulti

.SECONDEXPANSION:

all: $(PROGS)

atodtst: $(APPS)/$$@/$$@.c obj/atod20.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/atod20.o obj/findtype.o $(ONEWIREOBJS)

counter: $(APPS)/$$@/$$@.c obj/cnt1d.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/cnt1d.o obj/findtype.o $(ONEWIREOBJS)

coupler: $(APPS)/$$@/$$@.c obj/swt1f.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/swt1f.o obj/findtype.o $(ONEWIREOBJS)

debit: $(APPS)/shadebit/shademo.c obj/shadebit.o $(SHAOBJS)
	$(CC) $(LFLAGS) $< obj/shadebit.o $(SHAOBJS)

debitvm: $(APPS)/shadebit/shademo.c obj/shadbtvm.o $(SHAOBJS)
	$(CC) -DCOPRVM $(LFLAGS) $< obj/shadbtvm.o $(SHAOBJS)

fish: $(APPS)/$$@/$$@.c $(FILEOBJS)
	$(CC) $(LFLAGS) $< $(FILEOBJS)

gethumd: $(APPS)/$$@/$$@.c obj/atod26.o obj/screenio.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/atod26.o obj/screenio.o obj/findtype.o $(ONEWIREOBJS)

humalog: $(APPS)/$$@/$$@.c obj/findtype.o obj/humutil.o obj/mbee77.o obj/mbscrx77.o obj/pw77.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) -lm $< obj/findtype.o obj/humutil.o obj/mbee77.o obj/mbscrx77.o obj/pw77.o $(ONEWIREOBJS)

initcopr: $(APPS)/shadebit/shacopr.c obj/shadebit.o $(SHAOBJS)
	$(CC) $(LFLAGS) $< obj/shadebit.o $(SHAOBJS)

initcoprvm: $(APPS)/shadebit/shacopr.c obj/shadbtvm.o $(SHAOBJS)
	$(CC) -DCOPRVM $(LFLAGS) $< obj/shadbtvm.o $(SHAOBJS)

initrov: $(APPS)/shadebit/sharov.c obj/shadebit.o $(SHAOBJS)
	$(CC) $(LFLAGS) $< obj/shadebit.o $(SHAOBJS)

initrovvm: $(APPS)/shadebit/sharov.c obj/shadbtvm.o $(SHAOBJS)
	$(CC) -DCOPRVM $(LFLAGS) $< obj/shadbtvm.o $(SHAOBJS)

memutil: $(APPS)/$$@/$$@.c $(FILEOBJS)
	$(CC) $(LFLAGS) $< $(FILEOBJS)

mweather: $(APPS)/$$@/$$@.c obj/findtype.o $(WEATHEROBJS)
	$(CC) $(LFLAGS) $< obj/findtype.o $(WEATHEROBJS)

ps_check: $(APPS)/sa_ps/$$@.c obj/ps02.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/ps02.o obj/findtype.o $(ONEWIREOBJS)

ps_init: $(APPS)/sa_ps/$$@.c obj/ps02.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/ps02.o obj/findtype.o $(ONEWIREOBJS)

sha_chck: $(APPS)/sa_sha/$$@.c obj/shadbtvm.o $(SHAOBJS)
	$(CC) $(LFLAGS) $< obj/shadbtvm.o $(SHAOBJS)

sha_init: $(APPS)/sa_sha/$$@.c obj/shadbtvm.o $(SHAOBJS)
	$(CC) $(LFLAGS) $< obj/shadbtvm.o $(SHAOBJS)

shaapp: $(APPS)/$$@/$$@.c obj/ibsha33o.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/ibsha33o.o $(ONEWIREOBJS)

swtduo: $(APPS)/$$@/$$@.c obj/swt3A.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/swt3A.o $(ONEWIREOBJS)

swtloop: $(APPS)/$$@/$$@.c obj/swt12.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/swt12.o obj/findtype.o $(ONEWIREOBJS)

swtmain1c: $(APPS)/$$@/$$@.c obj/swt1C.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/swt1C.o obj/findtype.o $(ONEWIREOBJS)

swtmulti: $(APPS)/$$@/$$@.c obj/swt29.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/swt29.o obj/findtype.o $(ONEWIREOBJS)

swtsngl: $(APPS)/$$@/$$@.c obj/swt05.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/swt05.o obj/findtype.o $(ONEWIREOBJS)

temp: $(APPS)/$$@/$$@.c obj/temp10.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/temp10.o obj/findtype.o $(ONEWIREOBJS)

thermodl: $(APPS)/thermo/$$@.c obj/thermo21.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/thermo21.o obj/findtype.o $(ONEWIREOBJS)

thermoms: $(APPS)/thermo/$$@.c obj/thermo21.o obj/findtype.o $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< obj/thermo21.o obj/findtype.o $(ONEWIREOBJS)

tm_check: $(APPS)/sa_time/$$@.c obj/time04.o obj/findtype.o $(FILEOBJS)
	$(CC) $(LFLAGS) $< obj/time04.o obj/findtype.o $(FILEOBJS)

tm_init: $(APPS)/sa_time/$$@.c obj/time04.o obj/findtype.o $(FILEOBJS)
	$(CC) $(LFLAGS) $< obj/time04.o obj/findtype.o $(FILEOBJS)

tstfind: $(APPS)/$$@/$$@.c $(ONEWIREOBJS)
	$(CC) $(LFLAGS) $< $(ONEWIREOBJS)

obj/touch:
	mkdir -p obj
	touch obj/touch

obj/%.o: $(COMMON)/%.c obj/touch
	$(CC) $(CFLAGS) -o obj/$*.o $<

obj/owlinkobjs: $(LINK)/*.c $(SHARED)/*.c obj/owerr.o obj/ioutil.o obj/crcutil.o obj/touch
	$(CC) $(CFLAGS) $(LINK)/*.c $(SHARED)/*.c
	$(AR) r obj/owlinkobjs *.o obj/owerr.o obj/ioutil.o obj/crcutil.o
	rm -f *.o

clean:
	rm -rf obj
	rm -f *.o
	rm -f ${PROGS}

