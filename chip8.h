#ifndef chip_8_h
#define chip_8_h

class Chip8{
private:
    unsigned short opcode;

    unsigned char memory[4096]; //4K MEMORY
    unsigned char V[16]; //registers

    unsigned short I; //index register
    unsigned short pc; //program counter

    unsigned char delay_timer; //timer registers
    unsigned char sound_timer;

    unsigned short stack[16]; //stack
    unsigned short sptr; //stack pointer

public:
    unsigned char kypd[16]; //hex based keypad
    unsigned char gfx[64 * 32]; //graphic system
    Chip8();
    ~Chip8();

    bool drawFlag;
    void initialize();
    bool load();
    void emulateCycle();
    
}











#endif