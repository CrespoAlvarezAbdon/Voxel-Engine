# Deep Dive Open Engine.

## Introduction.
 Custom 3D grahpics engine written in C++ and OpenGL with the capability of letting
 the user define a custom game where the players are agents controlled by AI.
 Licensed under GNU General Public License Version 3.
 More details on LICENSE.txt file.

## Warranties.
 This project is in active development. In consequence, I do not offer any warranties regarding any code I upload related to this project or any other I have uploaded, I am uploading, and I will upload.
 For now, there is a release build compiled for Windows only. In case that build does not work or you do not use Linux, you have to compile the code yourself by downloading the source code from the release. Instructions are provided to do that correctly. 
 I do not take any responsibility regarding anything that could happen related to any code I upload.
 In case it is unclear. When you choose to download any code I upload, you make yourself
 responsible for anything that happens related to that code that I uploaded and you downloaded. 
 Download and use any code I upload at your own risk. I am not responsible for anything that happens to you if you download any code I upload.

## Purpose.
 The quick evolution of how artificial intelligences are developed in the
 recent years has made possible to greatly expand the types of tasks that they
 can perform. From trying to find an equation that is able to find a pattern
 based in some provided data to being able to understand the rules of a video
 game by learning. However, as the new types of jobs that an AI can learn
 to do are more and more complex, it may be difficult to observe its changing
 behaviour with just numbers and graphs. The purpose of this project is
 to deliver a operational 3D engine to visually represent simulated scenarios
 and obtain more significant information about how the AI is behaving when
 learning how to complete its assigned task, while at the same time leaving the
 engine’s code easy to understand and available for all users in order to help
 future programmers that are new either to graphics programming or the field
 of artificial intelligence.

## Installation guide.
A .pdf file with the installation guide is added to the repository and to every release.

## Usage guide.
**NOTE**. This guide is also provided in a .pdf file in the repository and in every release.

First of all, in order to start the engine, if the user wishes to launch it with
its Visual Studio project opened, it can be done by pressing the F5 key. That
will compile the project and launch it with the possibility of adding breakpoints
and other debug utilities offered by the IDE.
On the other hand, if the user desires to execute the engine without using
Visual Studio, then the user must navigate (assuming that the address starts
from the project’s root directory) to /x64 (or the type of system that the project
was compiled for by the user) and then, in the “Debug” directory, make sure that
the directories “AIData”, “records”, Resources” and “saves” are all updated (as,
for example, if the shader programs are missing, the engine will produce an error
when attempting to load them). Finally, in order to start the engine, execute the
“Deep dive open.exe” executable file.
Once the engine starts, by default, a command console will open featuring a
menu with three possible options. Enter 1 to start the example AI game and
access its main menu, 2 to enter the level editor mode and 3 to stop the engine’s
execution.
If the user chooses to enter the example AI game, another menu will be printed
to the console with 8 options, being the 8th one to exit the example AI game and
return to the previously mentioned menu. The first two options are for,
respectively, initiating agent training simulations with the first one generating the
AI agents with random weights (according to the parameters given to the AI game
in the code) and the other one loading some AI agents from an correct “.aidata”
file. Tampering with “.aidata” files is not supported and could lead to undefined
behaviour.
The next two options are the same as the first two ones except they are for
initiating testing simulations in which the AI agents are not trained and the only
output is some statistics of how well they performed when completing their tasks
When training or testing AI agents, the engine will ask for a number of
agents to create for the process. In training mode, said number must be even
due to the genetic operators used during the simulation. When loading
previously generated AI data, the engine will ask instead the path to its
corresponding file and how many agents to load from the file (if the user declares
to load “n” agents, the engine will load the first “n” agents found in the “.aidata”
file).
Once the number of AI agents is specified (and they are loaded into the engine
by using previously generated AI data), for training and testing modes, the engine
will ask for a number of epochs for the simulation. An epoch is one iteration of
the game or one game “match” in which the agents do their task. If there was an
AI game of football, an epoch would be considered a single match.
The 5th and 6th options are for generating a record of one epoch with,
respectively, AI agents with randomly generated weights or with agents that are
loaded from a “.aidata” file. When selecting one of these options, the engine will
first ask for the name of the “.rec” file that will be generated containing the record
of the match.
The “.aidata” and “.rec” files are stored, respectively, in the “AIData” and
“records” directories, found in the same directory where “Deep dive open.exe” is
found. Inside those directories, a folder is created for each registered AI game in
order not to mix files from diferent AI games as the definitions of AI data and AI
actions may vary per AI games.
If the record’s name corresponds to one already existing in the AI game’s
record file, then the engine will ask if the user desires to override the existing file
or not. If not, then it will ask for another name.
The 7th option is for playing a previously generated record. The engine will
ask the user the name of said record and, if found, it will start the engine’s
graphical mode, load the level that is associated with said recording (inside the
/saves/recordingsWorlds directory found in the same folder as where the “Deep
dive open.exe” file is located).