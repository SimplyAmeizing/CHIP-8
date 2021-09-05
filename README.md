CHIP-8 Emulator

In order to get this CHIP-8 Emulator up and running, first you have to be on a Windows or Linux OS.

Then, make sure that minGW is installed and can be utilized within the CLI. Download all the files.

Navigating to the correct directory and then running the command: g++ -ISDL2/include -LSDL2/lib -o main main.cpp chip8.cpp -lmingw32 -lSDL2main -lSDL2
on your CLI will get the code compiled.

Finally, to run a game on the emulator, we will utilize the command: ./main ROM/{game_of_choice}