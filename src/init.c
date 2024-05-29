#include <stdio.h>
#include <SDL2/SDL.h>
#include "init.h"


int init()
{
  if (!init_SDL())
  {
    return -1; 
  }
}

int init_SDL()
{

}
