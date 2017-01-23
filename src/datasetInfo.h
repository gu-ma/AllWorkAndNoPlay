//
//  DatasetInfo.h
//  KPIV-StuckInTraining
//
//  Created by Guillaume on 11.01.17.
//
//

#ifndef datasetInfo_h
#define datasetInfo_h

#include "ofMain.h"

class datasetInfo {
    
public:
    
    int playbackRate, pos;
    string poem;
    string fileName;

    datasetInfo( int _playbackRate, int _pos, string _poem, string _fileName ) {
        playbackRate = _playbackRate;
        pos = _pos;
        poem = _poem;
        fileName = _fileName;
    }
    
};

#endif /* datasetInfo_h */
