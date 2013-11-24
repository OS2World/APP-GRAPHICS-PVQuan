			ANIMDAT 1.1			Todd Sankey

  The source code for ANIMDAT is provided for use, modification,
education, etc. The author retains the rights to this project, however,
and simply asks that any significant changes or improvements
be reported to him. This software is supplied free by the author and
no other party shall charge for it. In addition, this package can
be freely distributed, but all parts must be included and credit
must be given to the author for use.

  Parts of the source code, in particular the scanner module, are
based heavily on the book Writing Compilers and Interpreters: An
Applied Approach by Ronald Mak, copyright 1991 published by John
Wiley and Sons.

  Included files:
README.DOC - 	This file, silly.
ANIMDAT.DOC -	A brief description on how to use ANIMDAT.
COMMON.H -	Global variables used in all modules.
ERROR.H -	Header for standardized method of error handling.
ERROR.C -	Routine for identifying and locating errors.
SCANNER.H -	Prototypes for interfacing with scanner module.
SCANNER.C -	Code for tokenizing a text buffer.
BTREE.H -	Prototypes for creating and evaluating mathematical expressions.
BTREE.C -	Code to create and traverse binary trees.
SYMTAB.H -	Symbol table interface.
SYMTAB.C -	Code for maintaining a symbol table
ANIMDAT.C -	Main program file.
ANIMDAT.EXE -	A compiled copy. Should work on any PC.
BBALL.DAT -	Template file for creating animation of a bouncing ball.
BBALL.VAR -	Variable definition file for bouncing ball.
MAKEFILE -	Project makefile compatible with Borland C++ 2.0

  The source was compiles using Borland C++ 2.0. I have included the
makefile for people with compatible compilers. All you should have to
do is change the name of the compiler executable ( 'bcc' for the author )
in the makefile and choose the appropriate flags.

  If you have any problems, question, suggestion, or comments please
contact the author at sankey@unixg.ubc.ca .


					Todd Sankey
					sankey@unixg.ubc.ca
