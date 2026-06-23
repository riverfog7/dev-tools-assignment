< Project Environment >
- openFrameworks 0.12.1
- Windows / MSYS2 MinGW64
- C++20
- Build system: openFrameworks Makefile
- External libraries: none
  - MD5 checksum code is included locally in src/md5.h

< Steps to Run >
1. Install openFrameworks 0.12.1 for MSYS2:
   https://openframeworks.cc/setup/msys2/

2. Place or extract this project folder here:
   <of_root>/apps/Projects/dev-tools-assignment/

3. Open the MSYS2 MinGW64 terminal.

4. Move to the project folder:
   cd <of_root>/apps/Projects/dev-tools-assignment

5. Build the project:
   make Debug

6. Run the project:
   make RunDebug

< Notes >
- The project uses C++20 features such as std::bit_cast.
- config.make sets the project compile option to C++20.
- If the project does not rebuild after changing compile options, run:
  make clean
  make Debug
