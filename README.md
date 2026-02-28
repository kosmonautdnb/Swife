# Swife
A tiny and fastcoded swift and DOS based 3D level editor (in development). 

![ScreenShot](https://raw.github.com/kosmonautdnb/OpenGLDemoGame_DOS/main/SWIFE.PNG)

## Compiling  
It is created with WatcomC++ 11.0 (from 1997), but can also be compiled with OpenWatcom and DJGPP. It's purely coded on a bare metal DOS system using VCode as text editor. You may compile it with the provided !MAKE.BAT but maybe it's easier to use:  
- !DJGPP.BAT for a DJGPP build
- !WATCOM.BAT for a WatcomC / OpenWatcom build

## Disclaimer  
Don't try this with DosBOX or DosBOX-X this editor heavily uses floating point and by all means will be too slow in an emulation. 

## FreeDos  
You could install FreeDos on an USB stick using the application "Rufus". But be aware that USB sticks wear out if you do heavy/alot of saving. Better use a real (max 2TB) USB hard drive, you can use Rufus with it's "special" option enabled to do this (format 2TB and "burn" FreeDOS onto the hard drive). Also it's good to install SBEMUv1.0.0beta5 to have it's memory manager (DPMI host). FreeDOS doesn't run (easily) on UEFI Bios systems (you would need CSMWRAP for this). And it seems even if you run FreeDOS from an USB in seldom cases the Graphicsmode of Swife doesn't work (we do have an EGA fallback, though). Ah and finally you need a good PC, I think a Pentium 200 may not do.

## What does "Swife" do?
If you simply want to place objects or billboards/sprites in a 3D scene, the freeware scene currently offers limited options. Godot/Unreal/Blender/Unity (or perhaps you're familiar with the Sony Level Editor) are examples of such programs. Here's a DOS/FreeDOS option for placing 3D objects in a 3D scene. It already offers many features and will be significantly expanded in the future. In fact, you can use this tool to create your 3D scenes right now. To have more than 32 MB of usable memory, you should use CWSDPMI or a similar program (SBEMU 1.1v5 includes a good DPMI host).

A simple "description" video on YouTube: https://youtu.be/mZzvaHuNcAc?si=wcby2ptquoGMkwTJ

### (c)2026 by Stefan Mader
