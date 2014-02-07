                         1-Wire Public Domain Kit

                         USerial Build for Linux
                           Version 3.10 3/24/06

Introduction
------------

   This port was targeted for and tested on a RedHat Linux X86 machine.
   The provided makefile will build all of the example programs distributed
   in the 3.00 1-Wire Public Domain release.

   For documentation on the examples and the 1-Wire Public Domain
   kit in general please see the 'main' kit:
      http://www.maxim-ic.com/products/ibutton/software/1wire/wirekit.cfm

Examples
--------
   atodtst
   counter
   coupler
   debit
   debitvm
   fish
   gethumd
   humalog
   initcopr
   initcoprvm
   initrov
   initrovvm
   memutil
   mweather
   ps_check
   ps_init
   sha_chck
   sha_init
   shaapp
   swtduo
   swtloop
   swtmulti
   swtsngl
   temp
   thermodl
   thermoms
   tm_check
   tm_init
   tstfind

Tool Versions
-------------

   $ make -version
   GNU Make 3.80
   Copyright (C) 2002  Free Software Foundation, Inc.
   This is free software; see the source for copying conditions.
   There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE.

   $ gcc -dumpversion
   4.0.0

   $ gcc -dumpmachine
   i386-redhat-linux

General Notes
-------------

   - Relevant 1-Wire Information:
     http://www.maxim-ic.com/

   - 1-Wire Discussion Forum:
     http://discuss.dalsemi.com/
