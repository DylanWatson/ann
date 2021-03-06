#include "neuralNetwork.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cmath>

using namespace std;

float digit_rounding(float num) {
    return (floor(num*100000))/100000;
}

neuralNetwork::neuralNetwork() {
    //Do stuff here!
}


neuralNetwork::~neuralNetwork() {
    //Delete stuff here!
    //delete all the inNodes
    for(int i = 0; i < numInNodes; i++)
        delete inNodes[i];
    //delete all hidden nodes
    for(int i = 0; i < numHiddenNodes; i++) {
        delete hiddenNodes[i];
        delete [] inWeights[i];
    }
    //delete all the outNodes
    for(int i = 0; i < numOutNodes; i++) {
        delete outNodes[i];
        delete [] hiddenWeights[i];
    }
    if(debug == true) {
        //delete the training set
        for(int i = 0; i < numCorrectPatterns; i++) {
            delete [] correct[i];
            delete [] error[i];
        }
    }
    //delete all the answers, both for output and hidden
    for(int i = 0; i < numPatterns; i++) {
        delete [] patterns[i];
        delete [] outAnswers[i];
        delete [] hiddenAnswers[i];
    }
}


void neuralNetwork::run(int flag) {
    
    //determine whether to run the
    //correction code or not
    if(flag == 1) debug = true;
    else          debug = false;
    
    fileRead();
    
    //allocate the appropriate space
    if(!createNodes()) exit(-3);
    
    int generation = 0;
    
    do {
        runData();
        
        if(debug) {
            calculateError();
            
            updateHiddenWeights();
            updateInputWeights();
            
            updateNodeWeights();
        }
        
        if(generation % 1000 == 0)
            cout << generation << ": " << systemError << endl;
        generation++;
        
    } while(systemError > 0.000000001);
    
    cout << "\n\nCorrect -- Goals\t|\tFinal -- Output" << endl;
    for(int i = 0; i < numPatterns; i++) {
        for(int j = 0; j < numOutNodes; j++) {
            cout << correct[i][j] << " ";
        }
        cout << "\t|\t";
        for(int j = 0; j < numOutNodes; j++) {
            cout << outAnswers[i][j] << " ";
        }
        cout << endl;
    }
    
    cout << "\n\nAll finished." << endl;
    cout << "Check output.out for the output." << endl;
    
}


void neuralNetwork::fileRead() {
    
    //ask the user which folder to look for the files
    //This allows for multiple test cases
    cout << "What folder? (no slash) --> ";
    cin >> folderName;

    string weightsFile = folderName + "/" + "weights.in";
    string patternsFile = folderName + "/" + "patterns.in";
    string correctFile = folderName + "/" + "correct.in";
    
    //this is a class variable, doesn't need to be declared again
    outputFile = folderName + "/" + "output.out";
    
    //read the data
    if(!readWeights(weightsFile)) exit(-1);
    if(!readInputs(patternsFile)) exit(-2);
    if(debug == true)
        if(!readCorrect(correctFile)) exit(-75);
    
    
    //float check the files are compatible
    if(numInNodes != numVals) {
        cout << "The number of values per pattern do\n"
             << "not match the number of input nodes.\n"
             << "As a result, we must abort." << endl;
        exit(-4);
    }
    
}


bool neuralNetwork::readWeights(string fname) {
    //open the file
    ifstream file;
    
    //open the file
    //If there is a problem,
    //return false
    file.open( fname.c_str() );
        if(file.fail()) {
            cout << "Error opening " << fname << endl;
            cout << "Terminating program" << endl;
            return false;
        }
    
    //read the first three numbers
    //and store them in variables
    file >> numInNodes;
    file >> numHiddenNodes;
    file >> numOutNodes;
    
    //allocate memory or return false
    //There should be hidden# + output# nodes
    //of rows in the array
    inWeights = new float* [numHiddenNodes];
        if(!inWeights) return false;
        
    // Generate the weights storage for the hidden nodes from input
    for(int i = 0; i < numHiddenNodes; i++) {
        inWeights[i] = new float[numInNodes];
            if(!inWeights[i]) return false;
            
        //now we loop through the rest of the data
        for(int j = 0; j < numInNodes; j++) {
            file >> inWeights[i][j];
        }
    }
    
    //allocate memory for the weights
    hiddenWeights = new float* [numOutNodes];
        if(!hiddenWeights) return false;
    
    for(int i = 0; i < numOutNodes; i++) {
        
        //allocate even more memory
        hiddenWeights[i] = new float[numHiddenNodes];
            if(!hiddenWeights[i]) return false;
        
        //loop through the number of columns
        //and stash the weight data
        for(int j = 0; j < numHiddenNodes; j++) {
            file >> hiddenWeights[i][j];
        }
    }
    
    //make sure to close the file before exiting!
    file.close();
    
    //once everything is finished, return true
    return true;
}


bool neuralNetwork::readInputs(string fname) {
    //open the file
    ifstream file;
    
    //If there is a problem,
    //return false
    file.open( fname.c_str() );
        if(file.fail()) {
            cout << "Error opening " << fname << endl;
            cout << "Terminating program" << endl;
            return false;
        }
    
    //read the first three numbers
    //and store them in variables
    file >> numPatterns;
    file >> numVals;
    file >> maxVal;
    
    //allocate memory or return false
    patterns = new float* [numPatterns];
        if(!patterns) return false;
        
    outAnswers = new float* [numPatterns];
        if(!outAnswers) return false;
        
    hiddenAnswers = new float* [numPatterns];
        if(!hiddenAnswers) return false;
    
    //loop through the number of rows
    for(int i = 0; i < numPatterns; i++) {
        
        //allocate even more memory
        patterns[i] = new float[numVals];
            if(!patterns[i]) return false;
            
        //no need to assign anything yet
        //just create the space for later
        outAnswers[i] = new float[numOutNodes];
            if(!outAnswers[i]) return false;
            
        hiddenAnswers[i] = new float[numHiddenNodes];
            if(!hiddenAnswers[i]) return false;
        
        //loop through the number of columns
        //and stash the weight data
        for(int j = 0; j < numVals; j++) {
            file >> patterns[i][j];
        }
    }
    
    //make sure to close the file before exiting!
    file.close();
    
    //once everything is finished, return true
    return true;
}

bool neuralNetwork::readCorrect(string fname) {
    //open the file
    ifstream file;
    
    //If there is a problem,
    //return false
    file.open( fname.c_str() );
        if(file.fail()) {
            cout << "Error opening " << fname << endl;
            cout << "Terminating program" << endl;
            return false;
        }
    
    //read the first three numbers
    //and store them in variables
    file >> numCorrectPatterns;
    file >> numCorrectOutNodes;
    
    if(numOutNodes != numCorrectOutNodes) {
        cout << "File asserts incompatible number of output nodes" << endl;
        cout << "Training set incompatible with current network..." << endl;
        cout << "Terminating program" << endl;
        return false;
    } else if(numCorrectPatterns != numPatterns) {
        cout << "Too few numbers of correct data for this set of patterns" << endl;
        cout << "Please create a comprehensive training set..." << endl;
        cout << "Terminating program" << endl;
        return false;
    }
    
    //allocate memory or return false
    correct = new float* [numCorrectPatterns];
        if(!correct) return false;
        
    //allocate memory for the error array
    error = new float*[numPatterns];
        if(!error) return false;
    
    //loop through the number of rows
    for(int i = 0; i < numCorrectPatterns; i++) {
        
        //allocate even more memory
        correct[i] = new float[numCorrectOutNodes];
            if(!correct[i]) return false;
            
        // For each of the output nodes
        error[i] = new float[numOutNodes];
            if(!error[i]) return false;
            
        //loop through the number of columns
        //and stash the weight data
        for(int j = 0; j < numCorrectOutNodes; j++) {
            file >> correct[i][j];
        }
    }
    
    //make sure to close the file before exiting!
    file.close();
    
    //once everything is finished, return true
    return true;
}


bool neuralNetwork::writeHeader(string fname) {
    //open the file
    ofstream file;

    file.open( fname.c_str(), ios::trunc);
        if(file.fail()) return false;
        
    //simply write the number of patterns checked
    file << numPatterns << "\n";
    file.close();
    return true;
}

bool neuralNetwork::writeResults(string fname) {
    //create the file
    ofstream file;
    
    //open the file in append mode
    // or return with false upon failure
    file.open( fname.c_str() , ios::app);
        if(file.fail()) return false;
        
    //write the data of all the output nodes
    for(int i = 0; i < numOutNodes; i++) {
        file << outNodes[i]->getValue() << " ";
    }
    
    //add a new line for clarity
    file << "\n";
    file.close();
    return true;
}


bool neuralNetwork::writeSystemError() {
    string fname = folderName + "/" + "error.out";
    ofstream file;
    
    file.open( fname.c_str() , ios::trunc);
        if(file.fail()) return false;
        
    file << digit_rounding(systemError) << "\n";
    
    for(int k = 0; k < numPatterns; k++) {
        for(int i = 0; i < numOutNodes; i++) {
            //set the number of digits to 5 for rounding
            file << digit_rounding(error[k][i]) << " ";
        }
        file << "\n";
    }
}


void neuralNetwork::runData() {
    
    //cout << "Writing the header..." << endl;
    if(!writeHeader(outputFile)) exit(-6);
    
    for(int i = 0; i < numPatterns; i++) {
        //update the input values with the
        // next in the pattern!
        updateNodes(patterns[i]);
        
        //calculate the patterns with the
        //i-th set of pattern data
        calculateNodes();
        
        //store the answers in the answers 2D array
        storeAnswers(i);
        
        if(!writeResults(outputFile)) exit(-99);
    }
}


bool neuralNetwork::createNodes() {
    //allocate the space
    //and create the appropriate nodes
    inNodes = new node*[numInNodes];
        if(!inNodes) return false;
        
    for(int i = 0; i < numInNodes; i++) {
        inNodes[i] = new node();
    }
    
    //allocate space for the hidden node layer
    hiddenNodes = new node* [numHiddenNodes];
        if(!hiddenNodes) return false;
        
    //create each one individually
    for(int i = 0; i < numHiddenNodes; i++) {
        hiddenNodes[i] = new node(numInNodes);
        
        //loop through and add the weights to
        //the node's array
        for(int j = 0; j < numInNodes; j++){
            hiddenNodes[i]->addWeight(inWeights[i][j]);
        }
    }
    
    //allocate the space
    outNodes = new node*[numOutNodes];
        if(!outNodes) return false;
        
    for(int i = 0; i < numOutNodes; i++) {
        //pass in the respective set of weights to the correct nodes
        outNodes[i] = new node(numHiddenNodes);
    
        for(int j = 0; j < numHiddenNodes; j++) {
            //access to the weights have to be offset by the number of hidden nodes
            outNodes[i]->addWeight(hiddenWeights[i][j]);
        }
    }

    return true;
}


void neuralNetwork::updateNodes(float *patternSet) {
    //We must divide by the maxVal in order to normalize the data
    for(int i = 0; i < numVals; i++) {
        //update the Node to the new value
        inNodes[i]->setValue(patternSet[i]/maxVal);
    }
}

void neuralNetwork::storeAnswers(int index) {
    //for all the output nodes
    for(int i = 0; i < numOutNodes; i++) {
        //set the answers array to the value of the outNode.value()
        outAnswers[index][i] = outNodes[i]->getValue();
    }
    
    //for all the hidden nodes
    for(int i = 0; i < numHiddenNodes; i++) {
        //set the value in the hiddenAnswer to the value
        //of the hidden nodes. Used in back propagation
        hiddenAnswers[index][i] = hiddenNodes[i]->getValue();
    }
}

void neuralNetwork::calculateNodes() {

    float sum;
    
    //first let's calculate the hidden nodes
    for(int i = 0; i < numHiddenNodes; i++) {
        //clear the junk data
        sum = 0;
        
        //for the number of values
        for(int j = 0; j < numInNodes; j++) {
	        
            //create two temp variables
            //useful for debugging
            float a = hiddenNodes[i]->getWeight(j);
            float b = inNodes[j]->getValue();

            //increase the set sum
            sum += a*b;
            
        }
        
        //sigmoid function
        sum = 1/(1+exp(-sum));
        
        //set the sum to the output node value
        hiddenNodes[i]->setValue(sum);
    }
    
    for(int i = 0; i < numOutNodes; i++) {
        sum = 0;  //zero out the sum
        
        for(int j = 0; j < numHiddenNodes; j++) {
            //create two temp variables
            //useful for debugging
            float a = outNodes[i]->getWeight(j);
            float b = hiddenNodes[j]->getValue();
       
            //increase the set sum
            sum += a*b;
            
        }
        
        //set the sum to the output node value
        outNodes[i]->setValue(sum);
    }
}


void neuralNetwork::calculateError() {
    float summation = 0;
    // For all the patterns calculated
    for (int k = 0; k < numCorrectPatterns; k++) {
        for(int i = 0; i < numOutNodes; i++) {
            //sigma-k-j in the book
            float a = correct[k][i] - outAnswers[k][i];
            error[k][i] = a;
            
            summation += pow(a,2);  //for error in the book
        }
    }
    
    systemError = summation * 0.5;
    //cout << "total system error = " << systemError << endl;
    
    writeSystemError();
}


void neuralNetwork::updateHiddenWeights() {
    
    float summation = 0;
    float alpha = 0.1;

    for(int j = 0; j < numOutNodes; j++) {
        for(int i = 0; i < numHiddenNodes; i++) {
            
            //for all the patterns
            for(int k = 0; k < numPatterns; k++) {
                
                //calculate the sigma value (pp. 185, equation 5.24b)
                //float sigma = correct[k][j] - outAnswers[k][j];
                //float sigma = error[k][j];
                
                //finally finish the derivative by multiplying by the
                //yield of hidden node at index i
                summation += -error[k][j] * hiddenAnswers[k][i];
            }
        
        //finally, the weight of index j, i is...
        hiddenWeights[j][i] -= alpha * summation;
        summation = 0;
        }
    }
}

void neuralNetwork::updateInputWeights() {
    float summation = 0;
    float beta = 0.1;
    float temp = 0;
    

    for(int i = 0; i < numHiddenNodes; i++) {
        for(int h = 0; h < numInNodes; h++) {
            for(int k = 0; k < numPatterns; k++) {
                temp = 0;  //zero it out
                
                // first part of sigmoid derivative
                float a = hiddenAnswers[k][i] * (1 - hiddenAnswers[k][i]);
                
                //perform the summation over all the original errors
                for(int j = 0; j < numOutNodes; j++) 
                   temp += error[k][j] * hiddenWeights[j][i];
                
                //summation for the final equation
                summation += -temp * a * patterns[k][h];
            }
            
            //adjust the old weight with the results of the summations
            inWeights[i][h] -= (beta * summation);
            
        }
    }
}

void neuralNetwork::updateNodeWeights() {
    //update all the hidden node weights for each input
    for(int i = 0; i < numHiddenNodes; i++) {
        hiddenNodes[i]->resetCurrentWeightCounter();
        for(int j = 0; j < numInNodes; j++) {
            hiddenNodes[i]->addWeight(inWeights[i][j]);
        }
    }
    //update all the output node weights for each hidden node
    for(int i = 0; i < numOutNodes; i++) {
        outNodes[i]->resetCurrentWeightCounter();
        for(int j = 0; j < numHiddenNodes; j++){
            outNodes[i]->addWeight(hiddenWeights[i][j]);
        }
    }
    
}