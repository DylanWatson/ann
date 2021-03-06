class node {
    public:
        //constructors and destructors
        //for the input Nodes
        node();
        //for the output nodes
        node(int inNumWeights/*, float *inWeights*/);
        ~node();
        
        //returns values
        //no direct access to member variables;
        void addWeight(float val);
        void setValue(float val);
        float getValue();
        float getWeight(const int i);
        void resetCurrentWeightCounter();
        
    private:
        
        float value;
        float *weights;
        int numWeights;
        int currentWeights;
};