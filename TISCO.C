
 /* This is a stand-alone "launcher" program.
    It runs the tisco compiler, prepares files, runs
    TASM and prgm86, etc.
    This file is compiled to tisco.exe, the other source
    files are compiled to tisco00.exe.
  */

#include <stdio.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>
#include <dir.h>


//#define DEBUG

#define MAX_PATH         (MAXDRIVE+MAXDIR+MAXFILE+MAXEXT)



char   actDir[MAX_PATH];
char   buf[8192];
char   asmfile[MAX_PATH], homefile[MAX_PATH], purefile[MAXFILE],
       homeDir[MAX_PATH], aux[MAX_PATH];
char   drv[MAXDRIVE], dir[MAXDIR], file[MAXFILE], ext[MAXEXT];
char   chdirdone = 0;




void erro (int c)
{
    switch (c)  {
      case 0 : break;
      case 1 : printf("Error executing tisco00.exe\n"); break;
      case 2 : printf("Error executing z80asm.exe\n"); break;
      case 3 : printf("Error executing prgm86.exe\n"); break;
      case 4 : printf("Arguments?\n"); break;
      case 5 : printf("Error in copy\n"); break;
      case 6 : printf("Failed moving *.86P to source dir\n"); break;
      case 7 : printf("Failed moving *.LST to source dir\n"); break;
      case 8 : printf("Failed geting actual dir\n"); break;

      case 10 : perror("Changing to BIN dir"); break;
    }

    if (chdirdone)  {
       sprintf(aux, "%s%s", purefile, ".asm");
       remove(aux);
       sprintf(aux, "%s%s", purefile, ".obj");
       remove(aux);
       remove(purefile);

       if (chdir(actDir))
          perror("Changing dir to source dir");
    }

    exit(c);
}



void usage_and_exit ()
{
    if (spawnlp(P_WAIT, "tisco00.exe", 0, 0))
       erro(1);

//    printf("Usage: tisco <C_file_name> [-o[0,1,2]] [-s] [-d] [-i<dir>] [-ja|jr|jo] [-h]\n");
    exit(0);
}




int file_copy (char *to, char *from)
{
     int   c, ret = 0;
     FILE  *ffrom = fopen(from, "rt");
     FILE  *fto = fopen(to, "wt");

     if ((!ffrom) || (!fto))
        return -1;

     while ((c = fgetc(ffrom)) != EOF)
       if (fputc(c, fto) == EOF)
          {  ret = -1; break;  }

     fclose(ffrom);
     fclose(fto);
     return ret;
}



char *sacad (char *f)
{
    char  *s = f;
    for (; *s; s++)  if (*s == '\\')  *s = '/';
    return f;
}



main (int argc, char **argv)
{
    char   dumm[16];
//    char **v = argv;
//    for (v=argv; *v; v++)
//        printf("%s\n", *v);

    if (argc < 2)
       usage_and_exit();

    // Get home dir
    fnsplit(*argv, drv, dir, file, ext);
    sprintf(homeDir, "%s%s", drv, dir);
    sacad(homeDir);
    // Save actual dir
    if (!getcwd(actDir, MAX_PATH))
       erro(8);

#ifdef DEBUG
    printf("Dir actual %s\nHome dir %s\n", actDir, homeDir);
#endif

    if (spawnvp(P_WAIT, "tisco00.exe", argv))
       erro(1);

#ifdef DEBUG
    printf("  Tisco00 ja' correu\n");
    fgets(dumm, sizeof(dumm), stdin);
#endif

    // Split file name components
    fnsplit(argv[1], drv, dir, purefile, ext);
    // Get asm file name
    fnmerge(asmfile, drv, dir, purefile, ".ASM");
    // Get asm file name into home dir
    sprintf(homefile, "%s%s%s", homeDir, purefile, ".ASM");

#ifdef DEBUG
    printf("  copy %s -> %s\n", asmfile, homefile);
#endif

    // Copy asm file to home dir
    if (file_copy(homefile, asmfile))
       erro(5);

#ifdef DEBUG
    printf("  Copiei\n");
    fgets(dumm, sizeof(dumm), stdin);
    printf("Dir actual %s\nHome dir %s\n", actDir, homeDir);
#endif

    if (chdir(homeDir))           // Mudar para a home dir
       erro(10);
    chdirdone = 1;

    sprintf(asmfile, "%s%s", purefile, ".ASM");
    if (spawnlp(P_WAIT, "z80asm.exe", "", "-80", "-b", asmfile, 0))
       erro(2);

#ifdef DEBUG
    printf("  TASM ja' correu\n");
    fgets(dumm, sizeof(dumm), stdin);
#endif

    sprintf(aux, "%s%s", purefile, ".obj");
    sprintf(buf, "ren %s %s", aux, purefile);
    system(buf);

//    printf("Nome p/ prgm86: %s\n", purefile);

    if (spawnl(P_WAIT, "prgm86.exe", "", purefile, 0))
       erro(3);

    printf("\n");

#ifdef DEBUG
    printf("  PRGM86 ja' correu\n");
    fgets(dumm, sizeof(dumm), stdin);
#endif

    // Passar o *.86P para a dir do *.C
    sprintf(buf, "%s.86p", purefile);
    sprintf(aux, "%s\\%s.86p", actDir, purefile);
    sacad(aux);
    if (rename(buf, aux))
       erro(6);

    // Passar o *.LST para a dir do *.C
    sprintf(buf, "%s.lst", purefile);
    sprintf(aux, "%s\\%s.lst", actDir, purefile);
    sacad(aux);
    if (rename(buf, aux))
       erro(7);

    erro(0);       // Normal exit
}




