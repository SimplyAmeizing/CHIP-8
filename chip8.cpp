#include <iostream>
#include <cstream>
#include <fstream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#include "chip8.h"


unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8() {}

void Chip8::initialize(){
    pc = 0x200; //pc starts at 0x200
    opcode = 0;
    I = 0;
    sptr = 0;

    for(int i=0; i<2048; i++){
        gfx[i] = 0 //clear display
    }

    for(int j=0; j<16; j++){
        stack[j] = 0; //clear stack
        V[j] = 0; //clear registers
        kypd[j] = 0; //clear keypad
    }

    for(int k=0; k<4096; k++){
        memory[k] = 0; //clear memory
    }

    for(int l=0; l<80; l++){
        memory[l] = chip8_fontset[l]; //load fontset
    }

    delay_timer =0;//reset timers
    sound_timer = 0;
}

bool Chip8::load(char const* rpath){
    initialize();

    std::ifstream file(rpath, std::ios::binary | std::ios::ate);

    if(!file.is_open()){
        return false;
    }

    std::streampos size = file.tellg();
    char* buffer = new char[size];

    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    if ((4096-512) > size){
        for(int i=0; i<size; i++){
            memory[512 + i] = buffer[i];
        }
        delete[] buffer;
    }
    
    else{
        return false;
    }

    return true;
}

void Chip8::emulateCycle(){
    opcode = memory[pc] << 8 | memory[pc+1]; //two byte opcode

    switch(opcode & 0xF000){//decode opcode
        
        case 0x0000://00E_

            switch (opcode & 0x000F){//F is filler for last digit
                
                case 0x0000: //00E0 clears screen
                    for(int i=0; i<2048; i++)
                        gfx[i] = 0;
                    drawFlag=true;
                    pc+=2;
                    break;
                

                case 0x000E: //00EE returns from subroutine
                    --sptr;
                    pc = stack[sptr];
                    pc += 2;
                
                default:
                    cout << "unknown opcode\n";
            }
            break;
        
        case 0x1000: //1NNN jumps to adress NNN
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: //2NNN calls subroutine at NNN
            stack[sptr] = pc;
            ++sptr;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000: //3XNN skips next instruction if VX=NN
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) //X is a 4 bit register identifier
                pc += 4;
            else
                pc += 2;
            break;
        
        case 0x4000: //4XNN skips next instruction if VX!=NN
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) //X is a 4 bit register identifier
                pc += 4;
            else
                pc += 2;
            break;

        case 0x5000: //5XYN skips next instruction if VX=VY
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) //X is a 4 bit register identifier
                pc += 4;
            else
                pc += 2;
            break;

        case 0x6000: //6XNN sets VX to NN
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x7000: //7XNN adds NN to VX
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;

        case 0x8000://lots of 8XY_ so, we need another switch
            switch(opcode & 0x000F){
                case 0x0000: //sets VX = VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0001: //sets VX to VX or VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                
                case 0x0002: //sets VX to VX and VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                
                case 0x0003: //sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                
                case 0x0004: //adds VY to VX with carry flag included (VF set to 1 with carry)
                    if ((V[(opcode & 0x00F0) >> 4] + V[(opcode & 0x0F00) >> 8]) > 0xFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0005: //VY is subtracted from VX with VF set to 0 when borrow
                    if(V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0006: //Stores LSB of VX in VF, then shifts VX right one
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;

                case 0x0007: //VX is set to VY - VX with borrows included
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc +=2;
                    break;

                case 0x000E: //stores MSB of VX in VF, then shifts VX left one
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7; //shift right seven to get MSB
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
					cout << "unknown opcode\n";
            }
            break;
        
        case 0x9000: //skips next instruction if Vx not equal to Vy
            if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;
        
        case 0xA000: //sets I to address NNN
            I = (opcode & 0x0FFF);
            pc += 2;
            break;
        
        case 0xB000: //BNNN jumps to address NNN + V0
            pc = (opcode & 0x0FFF) + V[0x0];
            break;
        
        case 0xC000: //CXNN sets VX to the results of bitwise and operation on random number
            V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            pc += 2;
            break;

        case 0xD000: //DXYN draws a sprite at specific location
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short h = opcode & 0x000F; //height of N
            unsigned short pix;

            V[0xF] = 0;
            for(int i=0; i<h; i++){
                pix = memory[I + i];

                for(int j=0; j<8; j++){
                    if((pix & (0x80 >> j)) != 0){//means that bit is on that specific pixel
                        if(gfx[(x + i + ((y + j) * 64))] == 1)
                            V[0xF] = 1;
                        gfx[(x + i + ((y + j) * 64))] ^= 1;//xor for drawing sprite
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;

        case 0xE000:
            switch(opcode & 0x00FF){
                case 0x009E://EX9E skips next instruction if key stored in VX pressed
                    if(kypd[V[(opcode & 0x0F00) >> 8]] != 0)
                        pc += 4;
                    else
                        pc += 2;
                    break;

                case 0x00A1://EXA1 skips next instruction if key stored in VX NOT pressed
                    if(kypd[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc += 4;
                    else
                        pc += 2;
                    break;
                    
                default:
					cout << "unknown opcode\n";
            }
            break;

        case 0xF000:
            switch(opcode & 0x00FF){ 
                case 0x0007://FX07 set VX to value of delay timer
                    V[(opcode & 0x0F00)] = delay_timer;
                    pc += 2;
                    break;
                
                case 0x000A://FX0A makes sure a key press is awaited, then stored in VX
                    bool press = false;
                    for(int i=0; i<16; i++){
                        if(kypd[i] != 0){
                            V[(opcode & 0x0F00) >> 8] = i;
                            press = true;
                        }
                    }
                    if (press == false)
                        pc -= 0;
                    else
                        pc += 2;
                    break;
                
                case 0x0015: //FX15 sets delay timer to VX
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x0018: //FX18 sets the sound timer to VX
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x001E: //FX1E adds VX to I, where VF not affected
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x0029: //FX29 sets I to location of sprite for character in VX
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;
                
                case 0x0033: //FX33 stores the binary coded decimal representation of VX
                    unsigned char val = V[(opcode & 0x0F00) >> 8];
                    memory[I + 2] = val % 10; //incase we have two digit number
                    val /= 10;

                    memory[I + 1] = val % 10;
                    val /= 10;

                    memory[I] = val % 10;
                    pc += 2;
                    break;

                case 0x0055: //FX55 stores V0 to VX in memory address starting at I with offset from I increased
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[I + i] = V[i];

                    pc += 2;
                    break;
                
                case 0x0066: //FX55 fills V0 to VX in memory address starting at I with offset from I increased
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        V[i] = memory[I + i];
                    
                    pc += 2;
                    break;

                default:
					cout << "unknown opcode\n";
            }
            break;
        
        default:
		    cout << "unknown opcode\n";
    }

    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0){
        if (sound_timer == 1)
            cout << "BEEP";
        sound_timer--;
    }
}

chip8::~chip8() {}