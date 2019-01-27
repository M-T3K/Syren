# Syren

## Welcome to Syren.
-------------------------------------------
Syren is a CLI General Purpose Dynamic Link Library (Dll) Injector coded from scratch in C++. The Injector itself was written with DOD (Data Oriented Design) in mind. This means that there are very few objects involved. A UI might be added in the future once all of the main features of Syren-CLI are implemented.

## List of Features

* Standard (Load Library) DLL Injection.
* Support for a variety of command line arguments i.e. Help Message when calling **Syren.exe -h**.

### High Priority Upcoming Features

* Code Refactoring. Fixing naming inconsistencies, minor errors.
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
