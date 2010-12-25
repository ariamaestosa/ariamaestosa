#ifndef __UNIT_TEST_UTIL_H__
#define __UNIT_TEST_UTIL_H__

#include "AriaCore.h"

namespace AriaMaestosa
{
    class Sequence;
    
    /**
      * A simple implementation of ICurrentSequenceProvider that is useful for unit tests
      */
    class TestSequenceProvider : public ICurrentSequenceProvider
    {
        Sequence* m_seq;
        
    public:
        
        TestSequenceProvider(Sequence* seq)
        {
            m_seq = seq;
        }
        
        virtual Sequence* getCurrentSequence()
        {
            return m_seq;
        }
        
        virtual GraphicalSequence* getCurrentGraphicalSequence()
        {
            return NULL;
        }
    };
        
}

#endif
