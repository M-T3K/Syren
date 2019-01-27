# Syren


Welcome to Syren.
-----------------------------------------------------------------------------------------
Syren is a CLI General Purpose Dynamic Link Library (Dll) Injector programmed from scratch in C++. Keep in mind this project uses several **Experimental** C++ features, such as the C++2a filesystem from **experimental/filesystem**. Make sure you have these features installed in Visual Studio. A UI version will probably be developed in the future. However, I believe that functionality should come before beauty, and thus the UI will only be developed once the rest of the features are done.

## List of Features

* Standard (Load Library) DLL Injection.

## Upcoming Features

* Additional CLI-Args like 
    * '-s': "Stable" flag that let's you run the code without Experimental C++ Features.
    * '-v': Short for Verbose, makes additional information print to the console.
    * '-d': Creates a dump file in case of a crash/error with valuable information to understand what things went wrong.
* Manual Mapping of Dll.
* More advanced Injection techniques.
* SyrenUI - An Alternative version with a cleaner User Interface, probably using Qt, or something else.

### High Priority

Currently, a rework of the command line argument parsing is high priority. It is poorly done, and it mixes experimental with stable C++ features to provide Syren with its functionality without offering the user a choice.

## F.A.Q.

**Q**: Why do you use Trailing Return Type (TRT) ?
**A**: TRT is a better alternative to traditional C/C++ function declaration syntax because it lets you to have a single, more consistent syntax in your projects, specially if you use Lambdas (Since they use this syntax too). Then, all the function names will start at exactly the same place (5th Character). **However**, this should only be done in projects over which you have full control; since most projects use the old syntax as a convention and it would be inappropriate to change it. Overall, you should stick to one of these syntaxes only. Also, [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae). Therefore, TRT improves your code readability when properly used.

**Q**: When is 'X' feature going to be ready?
**A**: I don't have much free time, and I've got many projects to finish. That being said, I'm still working on Syren, and new features will come when ready.

