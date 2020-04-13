
  // Simple DLL test
  // Compile with
  //   ..\>tisco USEDLL.C
  // It needs SIMPDLL to be in your calc


#include "stdio86.inc"


// This DLL declaration isn't in a separate file because
// is too simple and there are no other declarations

dll SIMPDLL {
  uint SIMPDLL () export 0;
  void Hello_from_DLL () export 1;
}


void main ()
{
     putstr("SIMPDLL version ");
     putuint(SIMPDLL());
     newline();
     Hello_from_DLL();
}

