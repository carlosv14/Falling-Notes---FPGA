#include "Notes.h"
#include "VGA.h"
Notes::Notes(int x, int y, int width, int height, unsigned char * img, char type)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->img = img;
    this->active = 1;
    this->type = type;
}

   
  unsigned char deleted[] = {
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,
  BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK
  };
  
void Notes::moveNote(){
  if(this->y<110)
    this->y++;
    if(this->y==110)
      this->active=0;
}


void Notes::drawPiece()
{	      
  //VGA.drawRect(5,5,10,10);	
  //VGA.writeArea(5,15,note_width,note_height,projectileOwl);
  //VGA.clearArea(x,y,width,height);
  VGA.setColor(BLACK);
  VGA.clearArea(x,0,0,y);
  VGA.writeArea(x,y,width,height,img);
}

void Notes::deletePiece(){
  VGA.writeArea(x,y,width,height,deleted);
  delete img;
}
Notes::~Notes()
{
    //dtor
    delete img;
}

