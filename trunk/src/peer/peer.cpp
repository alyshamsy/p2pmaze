#include <SDL_net.h>
#include <iostream>
#include <unistd.h>
#include "network_interface.h"
#include "message_handler.h"

using namespace std;

int main()
{
  SDLNet_Init();
  network_interface network(6666);
  message_handler handler;
  network.set_message_handler(&handler);
  network.begin();
  while(true)
    {
      sleep(10);
    }
  SDLNet_Quit();
  return 0;
}
