# ParamBlitz
ParamBlitz is a Splatoon 2 tool that extracts decrypted parameter hashes (alongside other things) from the game while gameplay is active. Currently, the built version is for Japanese v3.1.0, but should be able to be easily ported to other versions.

At the moment, this tool has ONLY been tested on Ryujinx, and may not work or require additional work to function on original hardware or Yuzu. Support will not be provided.

## How does this work?
In Splatoon 2, all called parameters go through the `Lp::Sys::ParamNode::keyHash(Lp::Sys::ParamNode *this)` function. This program uses that by taking the buffer data and sending it to a file right before it's hashed.

All the files are saved to the `bprmList.txt` on the SD card. Hashes are often sent hundreds of times to load everything, so this file may bloat a lot. Simply use `sort` and `uniq` in a Linux terminal to clean it up after usage (how I do it), or use another way to only get unique lines only.

## Porting to Other Versions
> [!WARNING]  
> This has not been tested outside of 3.1.0! Structs may have changed, but it's unlikely.

To port this tool to other versions, you will need the address of the instruction where the buffer is fully loaded. Before 3.1.0, you can simply look up the `Lp::Sys::ParamNode::keyHash` function's name as described above, but you will need to locate it manually for versions after, as Splatoon 2 removed symbols starting with version 3.2.0.

After you find the function, simply look before the hashing function at the end of the code. It will look something like this in versions that contain symbols (the address you are looking for is the BLR instruction here).
```asm
BLR             X8      ; sead::BufferedSafeStringBase<char>::assureTerminationImpl_(void)
LDR             X0, [SP,#0x130+var_120] ; this
BL              _ZN4sead9HashCRC3214calcStringHashEPKc ; sead::HashCRC32::calcStringHash(char const*)
``` 

After you have your new address, you can simply replace [this address in the code](https://github.com/ashbinary/ParamBlitz/blob/main/source/program/main.cpp#L99) with your own address, then recompile exlaunch.

## Credits
💖 [Shadów](https://x.com/shadowninja108) - Created exlaunch and mentored me through everything (and really all I have coded in general). I cannot overstate how awesome he is and how none of this would be possible without him.
