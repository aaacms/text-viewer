//==================================================//
//    Multi-Platform Console (MPC) - v4.6.1 @ 2010  //
//     UFSM - Federal University of Santa Maria     //
//==================================================//

#ifndef __APP_DEMO_H__
#define __APP_DEMO_H__

#include <string>

static const int APP_LINES = 35;
static const int APP_COLUMNS = 100;

void initMpc(void);
void initApp(void);

void mostraTexto(int l, int c, char *msg);


void displayApp(void);

void cbMouse(int l, int c, int button, int state);

void cbKeyboard(int key, int modifier, bool special, bool up);

void cbUpdate(void);


#endif // __APP_DEMO_H__
