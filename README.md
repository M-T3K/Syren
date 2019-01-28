# Syren

## Welcome to Syren.
-------------------------------------------
Syren is a CLI General Purpose Dynamic Link Library (Dll) Injector coded from scratch in C++. The Injector itself was written with DOD (Data Oriented Design) in mind. This means that there are very few objects involved. A UI might be added in the future once all of the main features of Syren-CLI are implemented.

## List of Features

* Standard (Load Library) DLL Injection.
* Support for a variety of command line arguments i.e. Help Message when calling **Syren.exe -h**.

### High Priority Upcoming Features

* Manual Mapping of Dll.

## Upcoming Features

* More advanced Injection techniques.
* SyrenUI - An Alternative version with a cleaner User Interface, probably using Qt, or something else.

## F.A.Q.

**Q**: Why do you use Trailing Return Type (TRT) ?

**A**: TRT is a better alternative to traditional C/C++ function declaration syntax because it lets you to have a single, more consistent syntax in your projects, specially if you use Lambdas (Since they use this syntax too). Then, all the function names will start at exactly the same place (5th Character). **However**, this should only be done in projects over which you have full control; since most projects use the old syntax as a convention and it would be inappropriate to change it. Overall, you should stick to one of these syntaxes only. Also, [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae). Therefore, TRT improves your code readability when properly used.

---

**Q**: When is 'X' feature going to be ready?

**A**: I don't have much free time, and I've got many projects to finish. That being said, I'm still working on Syren, and new features will come when ready.

---

**Q**: I want to contribute. How should I do it?

**A**: Via Pull Request. State exactly what you are trying to accomplish and write a summary of how you achieved it. Keep your code as readable and simple as possible. Use DOD.


## Contribution Guidelines

As you may have seen, the project is structured in a 'weird' way (Specially so if you have an OOP Background). This is because I'm designing the project with data on my mind. In other words, **Syren follows DOD**. When contributing, please, follow the following guidelines:

- About OOP:
  Objects are data structures upon which you have full control over their life cycle. If you do not need data, don't use objects: use a namespace. If you do not need to control their life cycle or it is a single thing, use a struct instead. A struct may use Constructors and Destructors if they are useful (They're just functions at the end of the day. Also, OOP projects are bloated with unnecessary Objects. Way too often in way too many projects there's way too many objects that make the code less readable, increase the amount of lines, and impact performance negatively (Objects have a pretty big overhead when accessing the heap one at a time). They should not be used lightly. OOP is useful if used correctly, but spamming Objects without thinking is not the way to go.

- About Functional Programming:
  There are times where functional programming has a time and a place. Syren is not a functional programming project. However, some features native to this paradigm are used (and encouraged!) such as lambdas and TRT syntax for function declaration. Lambdas are used to, essentially, nest functions in an elegant manner; since otherwise implementations of this feature are compiler dependent. For any other usage, treat them like a basic function (See the next section).

- About Functions and Methods:
  When thinking about creating a new function, ask yourself: "Will I need this code more than once?". If not, don't bother creating it. Functions and methods should be created with reusability in mind. If you only need something once, you should not make a function. Exceptions to this include things like overloads (which tend to be easy to implement), or the creation of functions that improve readability (such as having a different injection function for each type of injection). Regarding the stupid idea that functions should always be less than 15 lines of code: ignore this advice. This, more often than not, reduces readability. If your function does something and doesn't share any code with another function, don't split it into several functions. It is as I stated before - functions are used for reusability. 

- Regarding libraries: I do not like using third party libraries, specially if I'm not going to be using them. Don't use third party libraries unless you are certain you will use most of it.

- Summary: If you don't need an object, don't use it. If you don't need a function, don't create it. Keep your code small. Keep your code simple.

- Don't adapt the problem to your code. Adapt your code to the problem.

- https://cellperformance.beyond3d.com/articles/2008/03/three-big-lies.html

### Naming Conventions

- Variables:
  
    - Global Variables use Pascal Case. Example: **Flags, DumpFile** 
    - Local Variables use snake_case.
    - Member Variables (Attributes, fields) come with an m_ in front to denote membership (Though you shouldn't be using these much).

- Functions & Lambdas:
    
    - Snake case when composed of different words. Example: **validate_process(), file_exists()**
    - Lower case when name comes from an abbreviation. Example: **fprint()**  is short for File Print.
    - Additional capital letter if there is a descriptor. Example: **fprintfLn()** where **Ln** is a descriptor and stands for **Line**, or **injectLoadlibrary** where **Loadlibrary** is the descriptor and denotes the usage of LoadLibrary.
    - The reason Pascal Case is not used for functions is to create a clear division between Windows API function calls (which use Pascal Case) and the rest of the code.

- kebab-case makes my eyes *bleed*. Blame e-lisp.