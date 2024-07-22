#include "mpc_c.h"
#include "text-viewer.h"

int main(void)
{
   initMpc();

   mpcRun(50); //frames por segundo

   return 0;
}
