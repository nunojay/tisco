
  // Simple DLL
  // Compile with
  //      ..\>tisco SIMPDLL.C -d


uint SIMPDLL () export 0
{
    return 0*256 + 0;   // Version 0.0
}

void Hello_from_DLL () export 1
{
    char  *msg;
    msg = "Hello, from DLL!";
    getv  msg;
    asm "   call   _puts";
}


