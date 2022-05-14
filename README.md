# Android Anti-Cheat Engine 

Built in Visual Studio 2022

AARCH64 Only

It works with ring 3 privileges. (No ROOT Required.)

## Author
- codetronik (codetronik@gmail.com)
- Lyn Heo (https://github.com/lynheo)

## Features
Integrity
 - Check GOT section table Hooking
 - Check inline Hooking (memory address based)
 - Check code/rodata section Modification

Anti Reversing
 - Implementation of syscall API
 
Scanning tools 
 - Scanning the binary pattern via the process memory

Root Detection [todo]
 - Search for files related to magisk
 - Detect magisk-specific signatures
 - Detect adb root privileges

## Output
- dynamic library (libace.so / libsample.so)

## Test Environment
AARCH64 (ARM64)
- Galaxy Tab S6 Lite (Android 11)
- Galaxy Note 8 (Android 9)
