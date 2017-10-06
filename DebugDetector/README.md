DD - DebugDetector
============================

###Notes
- This tool should help to test if your debugger is invisible against the targets. Maybe some can learn something about the anti debug techniques while browsing the source. Of course the most things are easy to bypass but you should see what you need to fix in your debugger

- If you have some ideas, other techniques (no time related things currently) or other suggestions than feel free to contact me!

 * zer0fl4g[at]gmail[dot]com

###Ideas & Techniques & collected Links
* [CodeProject - an anti reverse engineering guide](http://www.codeproject.com/Articles/30815/An-Anti-Reverse-Engineering-Guide)     
* [Veracode - whitepaper_antidebugging.pdf](http://www.veracode.com/images/pdf/whitepaper_antidebugging.pdf)
* [Spareclockycles - stack necromancy defeating debuggers by raising the dead](http://spareclockcycles.org/2012/02/14/stack-necromancy-defeating-debuggers-by-raising-the-dead/)
* [Symantec - windows anti debug reference](http://www.symantec.com/connect/articles/windows-anti-debug-reference)
* [Tuts4You](http://tuts4you.com/download.php?view.3260)

###ToDo
+ add more Plugins
+ add bad driver names
+ fix x64 support
 + remove as much inline asm as possible
+ fix more memory leaks

####Changelog until now
+ added NtQuerySystemInformation
+ added NtSetDebugFilterState
+ fixed small memory leaks
+ changed version numbers (automatic set to build date of plugins)

####Changelog v0.2
+ added color on detection
+ added better error reporting by plugins
+ added OSVersion as parameter to plugins
+ added NtYieldExecution plugin (by Aguila)
+ added CheckHeapMemory plugin
+ added some new windows to the FindBadWindow plugin
+ fixed a Windows XP display problem

###Features
+ Plugin Interface
 + simple to use
 + error messages
+ Show percentage of detection
+ 20 Plugins
 + DebugObject
     + Using NtQueryInformationProcess to see if there are Debugging Objects for our process
 + CheckRemoteDebuggerPresent
     + simple api which checks if a debugger is present
 + HardwareBreakpoint
     + checking the current thread for breakpoints in CONTEXT.dr0 - .dr3
 + IsDebuggerPresent
     + simple api which checks if a debugger is present
 + MemoryBreakpoint
     + places a page guard and in case there is no exception we know that we are debugged
 + OpenCSRSS (doesn´t work anymore ?)
     + Opens a handle for csrss , should not be able to without DebuggingFlag
 + OutputDebugString
     + if eax == 1 we are in normal mode. else offset of string is found in eax
 + ParentProcess
     + normaly we run with explorer.exe as parent
 + ProcessDebugFlags
     + uses NtQueryInformationProcess to check if our process has debugflags
 + Unhandled Exception
     + raises a division by 0 exception and in case we are not debugged everything wents good since we capture the exception
 + PEB.BeingDebugged
     + Checks the Process Enviroment Block if the debugbit is set
 + PEB.GlobalFlags
     + checks if flag is set in PEB (`FLG_HEAP_ENABLE_TAIL_CHECK && FLG_HEAP_ENABLE_FREE_CHECK && FLG_HEAP_VALIDATE_PARAMETERS`)
 + PEB.ProcHeapFlag
     + checks if the ForceFlag is set in the PEB
 + FindBadProcesses
     + enums the process list and checks if given processes are running
 + FindBadWindows
     + enums the window list and checks if given windows are found
 + FindBadDrivers
     + enums the driver list and checks if given drivers are found
 + NtYieldExecution (by Aguila)
     + NtYieldExecution returns STATUS_NO_YIELD_PERFORMED if there is no other thread ( e.g debugger) but often fails if the system is overloaded and doesn´t allow a switch (not a good method)
 + CheckHeapMemory
     + allocs memory in the heap and checks if FEEEABABABABABABABABFEEE (exists only on debug mode as overflow detection) is there
 + NTSetDebugFilterState
	 + Uses the return value of the ntdll api "NtSetDebugFilterState" to check if the target is running under a debugger
 + NTQuerySystemInformation
	 + Uses the NtQuerySystemInformation API to check if the target is running under a debugger