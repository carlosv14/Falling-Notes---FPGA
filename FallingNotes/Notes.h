#ifndef NOTES_H
#define NOTES_H


class Notes
{
    public:
        char type;
        int exploted;
        int x,y,width,height;
        unsigned char * img;
        int active;
        Notes(int x, int y, int width, int height, unsigned char * img, char type);
        void moveNote();
        void drawPiece();
        void deletePiece();
        virtual ~Notes();
    protected:
    private:
};

#endif // NOTES_H

