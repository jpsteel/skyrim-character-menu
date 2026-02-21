#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <SKSE/SKSE.h>

void RevertCallback(SKSE::SerializationInterface*);
void SaveCallback(SKSE::SerializationInterface*);
void LoadCallback(SKSE::SerializationInterface*);

#endif  // SERIALIZATION_H
