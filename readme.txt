Doom port  for Genesis/MegaDrive. Based on linux doom sources.
use Mega EverDrive PRO to run it
doom*.wad file from original game required. Put it to the game folder. Teseted with wad files from doom and doom2

doom-68k.md: This version does not have any optimizations and not use fpga for acceleration.  Literally work on bare Genesis hardware as is.
 Extended-SSF mapper used for available RAM expansion.
 performance: one frame per 2-3 sec

doom-ed.md: This version has some optiomizations in code and use fpga for acceleration.
 Math operations (mul/div) and few heavy rendering operations was accelerated using fpga.
 8bit linear framebuffer transformation to 4-bit Genesis tile format by fpga "on the fly".
 performance: 1-2 fps

This demos completely unplayable due the low fps and lack of palette transformation. I believe it can be done much better if i would have infinity time for working over fun stuff like that, but i gonna stop at this point