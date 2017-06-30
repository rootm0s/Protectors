```
--------------------------------------------------------------------------------
Name....: x86 Virtualizer - source code
Author..: ReWolf
Date....: IV/V.2007
Rel.Date: VIII.2007

e.mail..: rewolf@rewolf.pl
www.....: http://blog.rewolf.pl
--------------------------------------------------------------------------------
```
Simple PE protector (x86) based on virtual machine.

#### **Principle of operation:** ####
![http://rewolf.pl/gfx/custom/x86.virt.before.gif](http://rewolf.pl/gfx/custom/x86.virt.before.gif)
![http://rewolf.pl/gfx/custom/x86.virt.after.gif](http://rewolf.pl/gfx/custom/x86.virt.after.gif)
```
--------------------------------------------------------------------------------

This product includes Length Dissasembler engine:

 *  Hacker Disassembler Engine
 *  Copyright (c) 2006-2007 Veacheslav Patkov

--------------------------------------------------------------------------------
Files:

\gpl.txt                           - GNU GPL license text
\bin\loader\meta.exe               - compiled loader
\bin\protector\x86.virt.exe        - compiled virtualizer
\bin\test_app\vm_test.exe          - compiled sample application
\bin\test_app\vm_test_vmed_01.exe  - sample app with one vm layer
\bin\test_app\vm_test_vmed_02.exe  - sample app with two vm layers
\doc\x86.virt.after.gif            - diagram - represents executable after virtualization
\doc\x86.virt.before.gif           - diagram - represents executable before virtualization
\doc\x86.virt.pdf                  - documentation
\src\loader\loader.asm             - loader source code
\src\protector\common.cpp          - some common functions
\src\protector\common.h            - header for above
\src\protector\hde.h               - header for Hacker Disassembler Engine
\src\protector\hde.lib             - Hacker Disassembler Engine library
\src\protector\macros.h            - auxiliary macros
\src\protector\main.cpp            - main program (gui, PE handling etc...)
\src\protector\poly_encdec.h       - binary version of poly_(enc/dec) function
\src\protector\protect.cpp         - core virtualization engine
\src\protector\protect.h           - header for above
\src\protector\res.rc              - resources
\src\protector\resource.h          - header for above
\src\test_app\main.cpp             - sample application
\src\test_app\res.rc               - resources
\src\test_app\resource.h           - header for above

--------------------------------------------------------------------------------
```
#### **Related links:** ####
  * http://blog.rewolf.pl/blog/?p=15
  * http://blog.rewolf.pl/blog/?p=22
  * http://rewolf.pl/stuff/x86.virt.pdf
  * http://rewolf.pl
