# Microsoft Developer Studio Project File - Name="tonclib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=tonclib - Win32 Clean
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tonclib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tonclib.mak" CFG="tonclib - Win32 Clean"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tonclib - Win32 Build" (based on "Win32 (x86) External Target")
!MESSAGE "tonclib - Win32 Clean" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "tonclib - Win32 Build"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tonclib_"
# PROP BASE Intermediate_Dir "tonclib_"
# PROP BASE Cmd_Line "make -f tonclib.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "tonclib.exe"
# PROP BASE Bsc_Name "tonclib.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "tonclib_"
# PROP Intermediate_Dir "tonclib_"
# PROP Cmd_Line "make -f tonclib.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "tonclib.exe"
# PROP Bsc_Name "tonclib.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "tonclib - Win32 Clean"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tonclib0"
# PROP BASE Intermediate_Dir "tonclib0"
# PROP BASE Cmd_Line "make -f tonclib.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "tonclib.exe"
# PROP BASE Bsc_Name "tonclib.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "tonclib0"
# PROP Intermediate_Dir "tonclib0"
# PROP Cmd_Line "make -f tonclib.mak clean"
# PROP Rebuild_Opt "/a"
# PROP Target_File "tonclib.exe"
# PROP Bsc_Name "tonclib.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "tonclib - Win32 Build"
# Name "tonclib - Win32 Clean"

!IF  "$(CFG)" == "tonclib - Win32 Build"

!ELSEIF  "$(CFG)" == "tonclib - Win32 Clean"

!ENDIF 

# Begin Group "hdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\affine.h
# End Source File
# Begin Source File

SOURCE=.\bg.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\core.h
# End Source File
# Begin Source File

SOURCE=.\divlut.h
# End Source File
# Begin Source File

SOURCE=.\geom.h
# End Source File
# Begin Source File

SOURCE=.\interrupt.h
# End Source File
# Begin Source File

SOURCE=.\keypad.h
# End Source File
# Begin Source File

SOURCE=.\luts.h
# End Source File
# Begin Source File

SOURCE=.\oam.h
# End Source File
# Begin Source File

SOURCE=.\regs.h
# End Source File
# Begin Source File

SOURCE=.\sinlut.h
# End Source File
# Begin Source File

SOURCE=.\swi.h
# End Source File
# Begin Source File

SOURCE=.\vid.h
# End Source File
# End Group
# Begin Group "src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\affine.c
# End Source File
# Begin Source File

SOURCE=.\bg.c
# End Source File
# Begin Source File

SOURCE=.\color.c
# End Source File
# Begin Source File

SOURCE=.\core.c
# End Source File
# Begin Source File

SOURCE=.\divlut.c
# End Source File
# Begin Source File

SOURCE=.\geom.c
# End Source File
# Begin Source File

SOURCE=.\interrupt.c
# End Source File
# Begin Source File

SOURCE=.\keypad.c
# End Source File
# Begin Source File

SOURCE=.\oam.c
# End Source File
# Begin Source File

SOURCE=.\sinlut.c
# End Source File
# Begin Source File

SOURCE=.\vid.c
# End Source File
# End Group
# Begin Group "asm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tonclib\single_ints.s
# End Source File
# Begin Source File

SOURCE=.\tonclib\swi.s
# End Source File
# End Group
# Begin Source File

SOURCE=.\base.h
# End Source File
# Begin Source File

SOURCE=.\tonc_gba.h
# End Source File
# Begin Source File

SOURCE=.\tonclib.mak
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# End Target
# End Project
