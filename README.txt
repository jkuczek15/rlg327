Assignment 1.10 - Choosing your own assignment -
=========
For this assignment, I decided to attempt to create an Android mobile application for our RLG 327 game. I have had very little experience creating Android applications in the past so this is a first time thing for me. To start out I had to figure out a way to compile C++ code on an Android device. There is a library called NDK which contains all the tools required to implement C++ within the Android application. 

The graphics and user interface for this assignment were mostly written in Java, along with some xml magic, however, Java needs to communicate with the C++ code in order for the program to work. 

In dungeon.h I implemented the singleton method that we learned in class in order to create a class dungeon_UI. This class contains mostly static methods (since there is only one instance) and it also contains a pointer to our dungeon structure. This class contains a method that converts our dungeons map of chars into a java string which can then be exported into our apps textview. One of the most difficult parts of this project was switching from using ncurses to just handling the inputs through buttons. Overall I learned a lot from this project.

It might be difficult for the TA's to compile my code since it was written in Android Studio and you need to install additional Android libraries in order to test the code. I would be happy to demo/explain any parts if neccessary to help the grading process. 

I also included screenshots of application running on my phone for a quick idea of what it looks like. All the main C++ code for the program is located under app/src/main/jni