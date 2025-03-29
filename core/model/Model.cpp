/*
* Model.h
* This file implements the Model class (a derived class of Asset),
* which is an abstract base class used to load a model and draw it.
*/

#include "Model.h"

// Static Protected Attributes
// ---------------------------
GLuint Model::nModels{}; // initialize the number of models in the scene to 0

// Constructors
// ------------
Model::Model(const std::string& name)
	: Asset(name, AssetType::MODEL) { nModels++; } // increments the number of models