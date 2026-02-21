#ifndef CLASSES_H
#define CLASSES_H

struct TESClass {
    std::string name;
    std::vector<std::string> majorSkills;
    std::string specialization;
    std::string description;
};

extern std::vector<TESClass> classicClasses;
extern std::vector<TESClass> classicClassesConstellations;
extern std::vector<TESClass> classicClassesFirmament;

#endif  // CLASSES_H