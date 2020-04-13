
  // Smallest Hello World! in TISCO
  // Compile with
  //     ..\>tisco shello.c -p


void puts();            // External declarations
void newline();


char  *string;

void main ()
{
    string = "Hello World!";
    getv string;      // Puts addr of string in hl
    puts();
    newline();
}



