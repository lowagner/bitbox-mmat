# Memory Matrix

A memorization game for the bitbox console, a low-power retro-modern open-hardware 
gaming platform, based on a Cortex M4 chip (STM32F407 specfically, with a clockrate
of 168MHz and 192KB of RAM) and absolutely no GPU.  I haven't tested this code
to see if it even runs on one, but you can play bitbox-mmat on a host computer.

![Picture of intro screen](https://github.com/lowagner.github.io/i/mmat.gif)

More info about the bitbox console here:

http://bitboxconsole.blogspot.com/

You can run this game on any host computer capable of compiling for the bitbox.  Get
the open-source bitbox SDK from here:

https://github.com/makapuf/bitbox

To get setup, you only need to set the environment variable BITBOX to the local path
of your bitbox SDK, and perhaps install a few things for cross compilation capabilities.

On Linux, follow this recipe to install the bitbox SDK:

http://bitboxconsole.blogspot.com/2013/09/tutorial-developing-for-bitbox-basics.html#more

On Windows:

http://bitboxconsole.blogspot.com/2014/08/developing-for-bitbox-under-windows.html

Then it should be a simple "make" from your `bitbox-mmat` directory, which will
create the bitbox executable and the emulated version for your host platform,
`mmat_emu`.


## Helpful links

bitbox on Github:

https://github.com/makapuf/bitbox

bitbox blog:  

http://bitboxconsole.blogspot.com/

bitbox Google group:

https://groups.google.com/forum/#!forum/bitbox-console


## Attribution / With thanks to

I used the bitbox-polar game as a template:

https://github.com/makapuf/bitbox-polar 

The song and sprites are my own, and licensed under 
the same GPL license found in this directory.  I used
this chiptune tracker to create the song:

https://github.com/lowagner/bitbox-chiptracker
